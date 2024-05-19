/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************/
UInt EtqFlagSwap(UInt flag) {
    UInt f = (flag & ~ETQ_LR);
    if (flag & ETQ_L)
        f |= ETQ_R;
    if (flag & ETQ_R)
        f |= ETQ_L;
    return f;
}
/******************************************************************************/
SIDE_TYPE GetSide(C VecI2 &edge, C VecI &tri) {
    Int e0 = edge.c[0], e1 = edge.c[1];
    if ((tri.c[0] == e0 && tri.c[1] == e1) || (tri.c[1] == e0 && tri.c[2] == e1) || (tri.c[2] == e0 && tri.c[0] == e1))
        return SIDE_R;
    if ((tri.c[0] == e1 && tri.c[1] == e0) || (tri.c[1] == e1 && tri.c[2] == e0) || (tri.c[2] == e1 && tri.c[0] == e0))
        return SIDE_L;
    return SIDE_NONE;
}
SIDE_TYPE GetSide(C VecI2 &edge, C VecI4 &quad) {
    Int e0 = edge.c[0], e1 = edge.c[1];
    if ((quad.c[0] == e0 && quad.c[1] == e1) || (quad.c[1] == e0 && quad.c[2] == e1) || (quad.c[2] == e0 && quad.c[3] == e1) || (quad.c[3] == e0 && quad.c[0] == e1))
        return SIDE_R;
    if ((quad.c[0] == e1 && quad.c[1] == e0) || (quad.c[1] == e1 && quad.c[2] == e0) || (quad.c[2] == e1 && quad.c[3] == e0) || (quad.c[3] == e1 && quad.c[0] == e0))
        return SIDE_L;
    return SIDE_NONE;
}
/******************************************************************************/
MeshBase::MeshBase(Int vtxs, Int edges, Int tris, Int quads, MESH_FLAG flag) : MeshBase() { create(vtxs, edges, tris, quads, flag); }
MeshBase::MeshBase(C MeshBase &src, MESH_FLAG flag_and) : MeshBase() { create(src, flag_and); }
MeshBase::MeshBase(C MeshRender &src, MESH_FLAG flag_and) : MeshBase() { create(src, flag_and); }
MeshBase::MeshBase(C MeshPart &src, MESH_FLAG flag_and) : MeshBase() { create(src, flag_and); }
MeshBase::MeshBase(C MeshLod &src, MESH_FLAG flag_and) : MeshBase() { create(src, flag_and); }
MeshBase::MeshBase(C PhysPart &src) : MeshBase() { create(src); }
/******************************************************************************/
void MeshBase::copyVtxs(C MeshBase &src) {
    Int elms = Min(vtxs(), src.vtxs());
    CopyN(vtx.pos(), src.vtx.pos(), elms);
    CopyN(vtx.nrm(), src.vtx.nrm(), elms);
    CopyN(vtx.tan(), src.vtx.tan(), elms);
    CopyN(vtx.bin(), src.vtx.bin(), elms);
    CopyN(vtx.hlp(), src.vtx.hlp(), elms);
    CopyN(vtx.tex0(), src.vtx.tex0(), elms);
    CopyN(vtx.tex1(), src.vtx.tex1(), elms);
    CopyN(vtx.tex2(), src.vtx.tex2(), elms);
    CopyN(vtx.tex3(), src.vtx.tex3(), elms);
    CopyN(vtx.matrix(), src.vtx.matrix(), elms);
    CopyN(vtx.blend(), src.vtx.blend(), elms);
    CopyN(vtx.size(), src.vtx.size(), elms);
    CopyN(vtx.material(), src.vtx.material(), elms);
    CopyN(vtx.color(), src.vtx.color(), elms);
    CopyN(vtx.flag(), src.vtx.flag(), elms);
    CopyN(vtx.dup(), src.vtx.dup(), elms);
}
void MeshBase::copyVtxs(C MeshBase &src, C CMemPtr<Bool> &is) {
    DEBUG_ASSERT(is.elms() <= src.vtxs() && CountIs(is) <= vtxs(), "copyVtxs");
    CopyIs(vtx.pos(), src.vtx.pos(), is);
    CopyIs(vtx.nrm(), src.vtx.nrm(), is);
    CopyIs(vtx.tan(), src.vtx.tan(), is);
    CopyIs(vtx.bin(), src.vtx.bin(), is);
    CopyIs(vtx.hlp(), src.vtx.hlp(), is);
    CopyIs(vtx.tex0(), src.vtx.tex0(), is);
    CopyIs(vtx.tex1(), src.vtx.tex1(), is);
    CopyIs(vtx.tex2(), src.vtx.tex2(), is);
    CopyIs(vtx.tex3(), src.vtx.tex3(), is);
    CopyIs(vtx.matrix(), src.vtx.matrix(), is);
    CopyIs(vtx.blend(), src.vtx.blend(), is);
    CopyIs(vtx.size(), src.vtx.size(), is);
    CopyIs(vtx.material(), src.vtx.material(), is);
    CopyIs(vtx.color(), src.vtx.color(), is);
    CopyIs(vtx.flag(), src.vtx.flag(), is);
    CopyIs(vtx.dup(), src.vtx.dup(), is);
}
void MeshBase::copyEdges(C MeshBase &src) {
    Int elms = Min(edges(), src.edges());
    CopyN(edge.ind(), src.edge.ind(), elms);
    CopyN(edge.adjFace(), src.edge.adjFace(), elms);
    CopyN(edge.nrm(), src.edge.nrm(), elms);
    CopyN(edge.flag(), src.edge.flag(), elms);
    CopyN(edge.id(), src.edge.id(), elms);
}
void MeshBase::copyEdges(C MeshBase &src, C CMemPtr<Bool> &is) {
    DEBUG_ASSERT(is.elms() <= src.edges() && CountIs(is) <= edges(), "copyEdges");
    CopyIs(edge.ind(), src.edge.ind(), is);
    CopyIs(edge.adjFace(), src.edge.adjFace(), is);
    CopyIs(edge.nrm(), src.edge.nrm(), is);
    CopyIs(edge.flag(), src.edge.flag(), is);
    CopyIs(edge.id(), src.edge.id(), is);
}
void MeshBase::copyTris(C MeshBase &src) {
    Int elms = Min(tris(), src.tris());
    CopyN(tri.ind(), src.tri.ind(), elms);
    CopyN(tri.adjFace(), src.tri.adjFace(), elms);
    CopyN(tri.adjEdge(), src.tri.adjEdge(), elms);
    CopyN(tri.nrm(), src.tri.nrm(), elms);
    CopyN(tri.flag(), src.tri.flag(), elms);
    CopyN(tri.id(), src.tri.id(), elms);
}
void MeshBase::copyTris(C MeshBase &src, C CMemPtr<Bool> &is) {
    DEBUG_ASSERT(is.elms() <= src.tris() && CountIs(is) <= tris(), "copyTris");
    CopyIs(tri.ind(), src.tri.ind(), is);
    CopyIs(tri.adjFace(), src.tri.adjFace(), is);
    CopyIs(tri.adjEdge(), src.tri.adjEdge(), is);
    CopyIs(tri.nrm(), src.tri.nrm(), is);
    CopyIs(tri.flag(), src.tri.flag(), is);
    CopyIs(tri.id(), src.tri.id(), is);
}
void MeshBase::copyQuads(C MeshBase &src) {
    Int elms = Min(quads(), src.quads());
    CopyN(quad.ind(), src.quad.ind(), elms);
    CopyN(quad.adjFace(), src.quad.adjFace(), elms);
    CopyN(quad.adjEdge(), src.quad.adjEdge(), elms);
    CopyN(quad.nrm(), src.quad.nrm(), elms);
    CopyN(quad.flag(), src.quad.flag(), elms);
    CopyN(quad.id(), src.quad.id(), elms);
}
void MeshBase::copyQuads(C MeshBase &src, C CMemPtr<Bool> &is) {
    DEBUG_ASSERT(is.elms() <= src.quads() && CountIs(is) <= quads(), "copyQuads");
    CopyIs(quad.ind(), src.quad.ind(), is);
    CopyIs(quad.adjFace(), src.quad.adjFace(), is);
    CopyIs(quad.adjEdge(), src.quad.adjEdge(), is);
    CopyIs(quad.nrm(), src.quad.nrm(), is);
    CopyIs(quad.flag(), src.quad.flag(), is);
    CopyIs(quad.id(), src.quad.id(), is);
}
/******************************************************************************/
void FixMatrixWeight(VecB4 &matrix, VecB4 &blend) {
    // merge duplicates (can happen due to bone remap)
    for (Int i = 3;;) {
        if (auto b = blend.c[i]) {
            auto m = matrix.c[i];
            REPD(j, i)
            if (matrix.c[j] == m) // if same matrix, no need to check if this one has blend
            {
                blend.c[j] += b; // merge into element closer to the start (because they should be sorted by weight)
                blend.c[i] = 0;  // clear
                matrix.c[i] = 0; // clear
                break;
            }
        }
        if (i <= 1)
            break; // no need to process #0 because inside the loop we compare it against previous elements only
        i--;
    }
    // sort
    REP(4 - 1) // 4 components -1 because we compare against next one below
    {
    again:
        if (blend.c[i] < blend.c[i + 1]) // weight order is wrong
        {
            Swap(blend.c[i], blend.c[i + 1]);
            Swap(matrix.c[i], matrix.c[i + 1]);
            if (i < 2) {
                i++;
                goto again;
            }                                    // check again pair from previous step
        } else if (blend.c[i] == blend.c[i + 1]) // if have same weight
            if (matrix.c[i] < matrix.c[i + 1])   // matrix order is wrong #SkinMatrixOrder
            {
                Swap(matrix.c[i], matrix.c[i + 1]);
                if (i < 2) {
                    i++;
                    goto again;
                } // check again pair from previous step
            }
    }
}
void FixMatrixWeight(VecUS4 &matrix, VecB4 &blend) {
    // merge duplicates (can happen due to bone remap)
    for (Int i = 3;;) {
        if (auto b = blend.c[i]) {
            auto m = matrix.c[i];
            REPD(j, i)
            if (matrix.c[j] == m) // if same matrix, no need to check if this one has blend
            {
                blend.c[j] += b; // merge into element closer to the start (because they should be sorted by weight)
                blend.c[i] = 0;  // clear
                matrix.c[i] = 0; // clear
                break;
            }
        }
        if (i <= 1)
            break; // no need to process #0 because inside the loop we compare it against previous elements only
        i--;
    }
    // sort
    REP(4 - 1) // 4 components -1 because we compare against next one below
    {
    again:
        if (blend.c[i] < blend.c[i + 1]) // weight order is wrong
        {
            Swap(blend.c[i], blend.c[i + 1]);
            Swap(matrix.c[i], matrix.c[i + 1]);
            if (i < 2) {
                i++;
                goto again;
            }                                    // check again pair from previous step
        } else if (blend.c[i] == blend.c[i + 1]) // if have same weight
            if (matrix.c[i] < matrix.c[i + 1])   // matrix order is wrong #SkinMatrixOrder
            {
                Swap(matrix.c[i], matrix.c[i + 1]);
                if (i < 2) {
                    i++;
                    goto again;
                } // check again pair from previous step
            }
    }
}
/******************************************************************************/
void SetSkin(C MemPtrN<IndexWeight, 256> &skin, VtxBone &matrix, VecB4 &blend, C Skeleton *skeleton) {
    const Int max_bone_influence = 4; // only 4 bones are supported

    MemtN<IndexWeight, 256> temp;
    temp = skin;

    // remove invalid indexes
    if (skeleton) {
        const Int max_index = skeleton->bones.elms() + VIRTUAL_ROOT_BONE;
        REPA(temp)
        if (!InRange(temp[i].index, max_index)) temp.remove(i);
    }

    // merge same bones (do this before removing empty references so we can have both positive and negative weights for the same matrix, used by the editor when changing skinning)
    REPA(temp) {
        C IndexWeight &a = temp[i];
        REPD(j, i) {
            IndexWeight &b = temp[j];
            if (a.index == b.index) {
                b.weight += a.weight;
                temp.remove(i);
                break;
            }
        }
    }

    // remove empty references (do this after merging the same bones)
    REPA(temp)
    if (temp[i].weight <= 0) temp.remove(i);

    // sort from most to least important
    temp.sort(Compare);

    // remove those that won't fit
    if (skeleton)                                                   // if skeleton is provided
        for (Int i = temp.elms() - 1; i >= max_bone_influence; i--) // iterate all weights that would get removed
        {
            Int bone = temp[i].index - VIRTUAL_ROOT_BONE,
                min_distance = BONE_NULL + 1,
                closest = 0;
            REPD(j, i) // get all closest bones
            {
                Int distance = skeleton->hierarchyDistance(bone, temp[j].index - VIRTUAL_ROOT_BONE);
                if (distance < min_distance) {
                    min_distance = distance;
                    closest = 1;
                } else // if found smaller distance than previous then set it, and set closest bones to 1
                    if (distance == min_distance) {
                        closest++;
                    } // if distance found is equal to min then another bone is close, so increase it by 1
            }
            if (closest) {
                Flt weight = temp[i].weight / closest;
                REPD(j, i)
                if (skeleton->hierarchyDistance(bone, temp[j].index - VIRTUAL_ROOT_BONE) == min_distance) temp[j].weight += weight; // add weight from this bone to all closest bones equally
            }
            temp.removeLast();
            if (closest)
                temp.sort(Compare); // remove last element and re-sort if any weights were modified
        }
    if (temp.elms() > max_bone_influence)
        temp.setNum(max_bone_influence); // this is needed because we're calculating sum below

    Flt sum = 0;
    REPA(temp)
    sum += temp[i].weight;
    if (sum) {
        sum = 255 / sum;
        REPAO(temp).weight *= sum; // normalize and scale to 255
        switch (temp.elms()) {
        case 0:
            goto zero; // shouldn't really happen

        case 1: {
            blend.set(255, 0, 0, 0); // 'blend.sum' must be exactly equal to 255 !!
            matrix.set(temp[0].index, 0, 0, 0);
        } break;

        case 2: {
            Byte w = RoundPos(temp[0].weight);
            blend.set(w, 255 - w, 0, 0); // 'blend.sum' must be exactly equal to 255 !!
            matrix.set(temp[0].index, temp[1].index, 0, 0);
        } break;

        case 3: {
            Byte w0 = RoundPos(temp[0].weight),
                 w1 = Mid(RoundPos(temp[1].weight), 0, 255 - w0); // limit to "255-w0" because we can't have "w0+w1>255"
            blend.set(w0, w1, 255 - w0 - w1, 0);                  // 'blend.sum' must be exactly equal to 255 !!
            matrix.set(temp[0].index, temp[1].index, temp[2].index, 0);
        } break;

        default: // 4 or more
        {
            Byte w0 = RoundPos(temp[0].weight),
                 w1 = Mid(RoundPos(temp[1].weight), 0, 255 - w0),     // limit to "255-w0   " because we can't have "w0+w1   >255"
                w2 = Mid(RoundPos(temp[2].weight), 0, 255 - w0 - w1); // limit to "255-w0-w1" because we can't have "w0+w1+w2>255"
            blend.set(w0, w1, w2, 255 - w0 - w1 - w2);                // 'blend.sum' must be exactly equal to 255 !!
            matrix.set(temp[0].index, temp[1].index, temp[2].index, temp[3].index);
        } break;
        }
        REPA(blend)
        if (!blend.c[i]) matrix.c[i] = 0; // clear bones to 0 if they have no weight
        // sort matrix/weight to list most important first, and in case weights are the same, then sort by matrix index (this is needed because even though 'temp' is already sorted, we need to sort again because weights now in byte format can be the same, and in which case we need to sort by matrix index), we do this, so in the future we can compare 2 matrix weights using fast checks like "matrix0==matrix1 && weight0==weight1" instead of checking each matrix index component separately (for cases where they are listed in different order)
        if (temp.elms() >= 3)
            FixMatrixWeight(matrix, blend); // need to check this only for 3 or more bones, because 1 and 2 will never have this (1 has always "255,0,0,0" weights, and 2 has always "w,255-w,0,0" weights, which means they are always different, because "Byte w" is always different than "255-w")
    } else {
    zero:
        blend.set(255, 0, 0, 0); // 'blend.sum' must be exactly equal to 255 !!
        matrix = 0;
    }
}
/******************************************************************************/
// VERTEX FULL
/******************************************************************************/
void VtxFull::reset() {
    Zero(T);
    color = WHITE;
    material.x = 255; // !! 'material.sum' must be exactly equal to 255 !!
    blend.x = 255;    // !!    'blend.sum' must be exactly equal to 255 !!
}
VtxFull &VtxFull::from(C MeshBase &mshb, Int i) {
    if (InRange(i, mshb.vtx)) {
        if (mshb.vtx.pos())
            pos = mshb.vtx.pos(i);
        else
            pos.zero();
        if (mshb.vtx.nrm())
            nrm = mshb.vtx.nrm(i);
        else
            nrm.zero();
        if (mshb.vtx.tan())
            tan = mshb.vtx.tan(i);
        else
            tan.zero();
        if (mshb.vtx.bin())
            bin = mshb.vtx.bin(i);
        else
            bin.zero();
        if (mshb.vtx.hlp())
            hlp = mshb.vtx.hlp(i);
        else
            hlp.zero();
        if (mshb.vtx.tex0())
            tex0 = mshb.vtx.tex0(i);
        else
            tex0.zero();
        if (mshb.vtx.tex1())
            tex1 = mshb.vtx.tex1(i);
        else
            tex1.zero();
        if (mshb.vtx.tex2())
            tex2 = mshb.vtx.tex2(i);
        else
            tex2.zero();
        if (mshb.vtx.tex3())
            tex3 = mshb.vtx.tex3(i);
        else
            tex3.zero();
        if (mshb.vtx.color())
            color = mshb.vtx.color(i);
        else
            color = WHITE;
        if (mshb.vtx.material())
            material = mshb.vtx.material(i);
        else
            material.set(255, 0, 0, 0); // !! 'material.sum' must be exactly equal to 255 !!
        if (mshb.vtx.matrix())
            matrix = mshb.vtx.matrix(i);
        else
            matrix.zero();
        if (mshb.vtx.blend())
            blend = mshb.vtx.blend(i);
        else
            blend.set(255, 0, 0, 0); // !!    'blend.sum' must be exactly equal to 255 !!
        if (mshb.vtx.size())
            size = mshb.vtx.size(i);
        else
            size = 0;
    } else
        reset();
    return T;
}
void VtxFull::to(MeshBase &mshb, Int i) C {
    if (InRange(i, mshb.vtx)) {
        if (mshb.vtx.pos())
            mshb.vtx.pos(i) = pos;
        if (mshb.vtx.nrm())
            mshb.vtx.nrm(i) = nrm;
        if (mshb.vtx.tan())
            mshb.vtx.tan(i) = tan;
        if (mshb.vtx.bin())
            mshb.vtx.bin(i) = bin;
        if (mshb.vtx.hlp())
            mshb.vtx.hlp(i) = hlp;
        if (mshb.vtx.tex0())
            mshb.vtx.tex0(i) = tex0;
        if (mshb.vtx.tex1())
            mshb.vtx.tex1(i) = tex1;
        if (mshb.vtx.tex2())
            mshb.vtx.tex2(i) = tex2;
        if (mshb.vtx.tex3())
            mshb.vtx.tex3(i) = tex3;
        if (mshb.vtx.color())
            mshb.vtx.color(i) = color;
        if (mshb.vtx.material())
            mshb.vtx.material(i) = material;
        if (mshb.vtx.matrix())
            mshb.vtx.matrix(i) = matrix;
        if (mshb.vtx.blend())
            mshb.vtx.blend(i) = blend;
        if (mshb.vtx.size())
            mshb.vtx.size(i) = size;
        if (mshb.vtx.dup())
            mshb.vtx.dup(i) = i;
    }
}
VtxFull &VtxFull::avg(C VtxFull &a, C VtxFull &b) {
    pos = Avg(a.pos, b.pos);
    nrm = !(a.nrm + b.nrm);
    tan = !(a.tan + b.tan);
    bin = !(a.bin + b.bin);
    hlp = Avg(a.hlp, b.hlp);
    tex0 = Avg(a.tex0, b.tex0);
    tex1 = Avg(a.tex1, b.tex1);
    tex2 = Avg(a.tex2, b.tex2);
    tex3 = Avg(a.tex3, b.tex3);
    size = Avg(a.size, b.size);
    color = Avg(a.color, b.color);
    material = AvgI(a.material, b.material);

    MemtN<IndexWeight, 256> skin;
    FREPA(a.matrix)
    skin.New().set(a.matrix.c[i], a.blend.c[i]);
    FREPA(b.matrix)
    skin.New().set(b.matrix.c[i], b.blend.c[i]);
    SetSkin(skin, matrix, blend, null);

    return T;
}
VtxFull &VtxFull::avg(C VtxFull &a, C VtxFull &b, C VtxFull &c) {
    pos = Avg(a.pos, b.pos, c.pos);
    nrm = !(a.nrm + b.nrm + c.nrm);
    tan = !(a.tan + b.tan + c.tan);
    bin = !(a.bin + b.bin + c.bin);
    hlp = Avg(a.hlp, b.hlp, c.hlp);
    tex0 = Avg(a.tex0, b.tex0, c.tex0);
    tex1 = Avg(a.tex1, b.tex1, c.tex1);
    tex2 = Avg(a.tex2, b.tex2, c.tex2);
    tex3 = Avg(a.tex3, b.tex3, c.tex3);
    size = Avg(a.size, b.size, c.size);
    color = Avg(a.color, b.color, c.color);
    material = AvgI(a.material, b.material, c.material);

    MemtN<IndexWeight, 256> skin;
    FREPA(a.matrix)
    skin.New().set(a.matrix.c[i], a.blend.c[i]);
    FREPA(b.matrix)
    skin.New().set(b.matrix.c[i], b.blend.c[i]);
    FREPA(c.matrix)
    skin.New().set(c.matrix.c[i], c.blend.c[i]);
    SetSkin(skin, matrix, blend, null);

    return T;
}
VtxFull &VtxFull::avg(C VtxFull &a, C VtxFull &b, C VtxFull &c, C VtxFull &d) {
    pos = Avg(a.pos, b.pos, c.pos, d.pos);
    nrm = !(a.nrm + b.nrm + c.nrm + d.nrm);
    tan = !(a.tan + b.tan + c.tan + d.tan);
    bin = !(a.bin + b.bin + c.bin + d.bin);
    hlp = Avg(a.hlp, b.hlp, c.hlp, d.hlp);
    tex0 = Avg(a.tex0, b.tex0, c.tex0, d.tex0);
    tex1 = Avg(a.tex1, b.tex1, c.tex1, d.tex1);
    tex2 = Avg(a.tex2, b.tex2, c.tex2, d.tex2);
    tex3 = Avg(a.tex3, b.tex3, c.tex3, d.tex3);
    size = Avg(a.size, b.size, c.size, d.size);
    color = Avg(a.color, b.color, c.color, d.color);
    material = AvgI(a.material, b.material, c.material, d.material);

    MemtN<IndexWeight, 256> skin;
    FREPA(a.matrix)
    skin.New().set(a.matrix.c[i], a.blend.c[i]);
    FREPA(b.matrix)
    skin.New().set(b.matrix.c[i], b.blend.c[i]);
    FREPA(c.matrix)
    skin.New().set(c.matrix.c[i], c.blend.c[i]);
    FREPA(d.matrix)
    skin.New().set(d.matrix.c[i], d.blend.c[i]);
    SetSkin(skin, matrix, blend, null);

    return T;
}
VtxFull &VtxFull::lerp(C VtxFull &a, C VtxFull &b, Flt step) {
    Flt step1 = 1 - step;
    pos = a.pos * step1 + b.pos * step;
    nrm = !(a.nrm * step1 + b.nrm * step);
    tan = !(a.tan * step1 + b.tan * step);
    bin = !(a.bin * step1 + b.bin * step);
    hlp = a.hlp * step1 + b.hlp * step;
    tex0 = a.tex0 * step1 + b.tex0 * step;
    tex1 = a.tex1 * step1 + b.tex1 * step;
    tex2 = a.tex2 * step1 + b.tex2 * step;
    tex3 = a.tex3 * step1 + b.tex3 * step;
    size = a.size * step1 + b.size * step;
    color = Lerp(a.color, b.color, step);
    material = Lerp(a.material, b.material, step);

    MemtN<IndexWeight, 256> skin;
    FREPA(a.matrix)
    skin.New().set(a.matrix.c[i], a.blend.c[i] * step1);
    FREPA(b.matrix)
    skin.New().set(b.matrix.c[i], b.blend.c[i] * step);
    SetSkin(skin, matrix, blend, null);

    return T;
}
VtxFull &VtxFull::lerp(C VtxFull &a, C VtxFull &b, C VtxFull &c, C Vec &blend) {
    Flt ba = blend.c[0],
        bb = blend.c[1],
        bc = blend.c[2];

    T.pos = a.pos * ba + b.pos * bb + c.pos * bc;
    T.nrm = !(a.nrm * ba + b.nrm * bb + c.nrm * bc);
    T.tan = !(a.tan * ba + b.tan * bb + c.tan * bc);
    T.bin = !(a.bin * ba + b.bin * bb + c.bin * bc);
    T.hlp = a.hlp * ba + b.hlp * bb + c.hlp * bc;
    T.tex0 = a.tex0 * ba + b.tex0 * bb + c.tex0 * bc;
    T.tex1 = a.tex1 * ba + b.tex1 * bb + c.tex1 * bc;
    T.tex2 = a.tex2 * ba + b.tex2 * bb + c.tex2 * bc;
    T.tex3 = a.tex3 * ba + b.tex3 * bb + c.tex3 * bc;
    T.size = a.size * ba + b.size * bb + c.size * bc;
    T.color = Lerp(a.color, b.color, c.color, blend);
    T.material = Lerp(a.material, b.material, c.material, blend);

    MemtN<IndexWeight, 256> skin;
    FREPA(a.matrix)
    skin.New().set(a.matrix.c[i], a.blend.c[i] * ba);
    FREPA(b.matrix)
    skin.New().set(b.matrix.c[i], b.blend.c[i] * bb);
    FREPA(c.matrix)
    skin.New().set(c.matrix.c[i], c.blend.c[i] * bc);
    SetSkin(skin, T.matrix, T.blend, null);

    return T;
}
VtxFull &VtxFull::mul(C Matrix &matrix, C Matrix3 &matrix3) {
    pos *= matrix;
    hlp *= matrix;
    nrm *= matrix3;
    tan *= matrix3;
    bin *= matrix3;
    return T;
}
/******************************************************************************/
// MESH BASE
/******************************************************************************/
MESH_FLAG MeshBase::flag() C {
    MESH_FLAG f = MESH_NONE;

    if (vtx.pos())
        f |= VTX_POS;
    if (vtx.nrm())
        f |= VTX_NRM;
    if (vtx.tan())
        f |= VTX_TAN;
    if (vtx.bin())
        f |= VTX_BIN;
    if (vtx.hlp())
        f |= VTX_HLP;
    if (vtx.tex0())
        f |= VTX_TEX0;
    if (vtx.tex1())
        f |= VTX_TEX1;
    if (vtx.tex2())
        f |= VTX_TEX2;
    if (vtx.tex3())
        f |= VTX_TEX3;
    if (vtx.matrix())
        f |= VTX_MATRIX;
    if (vtx.blend())
        f |= VTX_BLEND;
    if (vtx.size())
        f |= VTX_SIZE;
    if (vtx.material())
        f |= VTX_MATERIAL;
    if (vtx.color())
        f |= VTX_COLOR;
    if (vtx.flag())
        f |= VTX_FLAG;
    if (vtx.dup())
        f |= VTX_DUP;

    if (edge.ind())
        f |= EDGE_IND;
    if (edge.adjFace())
        f |= EDGE_ADJ_FACE;
    if (edge.nrm())
        f |= EDGE_NRM;
    if (edge.flag())
        f |= EDGE_FLAG;
    if (edge.id())
        f |= EDGE_ID;

    if (tri.ind())
        f |= TRI_IND;
    if (tri.adjFace())
        f |= TRI_ADJ_FACE;
    if (tri.adjEdge())
        f |= TRI_ADJ_EDGE;
    if (tri.nrm())
        f |= TRI_NRM;
    if (tri.flag())
        f |= TRI_FLAG;
    if (tri.id())
        f |= TRI_ID;

    if (quad.ind())
        f |= QUAD_IND;
    if (quad.adjFace())
        f |= QUAD_ADJ_FACE;
    if (quad.adjEdge())
        f |= QUAD_ADJ_EDGE;
    if (quad.nrm())
        f |= QUAD_NRM;
    if (quad.flag())
        f |= QUAD_FLAG;
    if (quad.id())
        f |= QUAD_ID;

    return f;
}
UInt MeshBase::memUsage() C {
    UInt size = 0;
    if (Int elms = vtx.elms()) {
        UInt s = 0;
        if (vtx.pos())
            s += SIZE(*vtx.pos());
        if (vtx.nrm())
            s += SIZE(*vtx.nrm());
        if (vtx.tan())
            s += SIZE(*vtx.tan());
        if (vtx.bin())
            s += SIZE(*vtx.bin());
        if (vtx.hlp())
            s += SIZE(*vtx.hlp());
        if (vtx.tex0())
            s += SIZE(*vtx.tex0());
        if (vtx.tex1())
            s += SIZE(*vtx.tex1());
        if (vtx.tex2())
            s += SIZE(*vtx.tex2());
        if (vtx.tex3())
            s += SIZE(*vtx.tex3());
        if (vtx.matrix())
            s += SIZE(*vtx.matrix());
        if (vtx.blend())
            s += SIZE(*vtx.blend());
        if (vtx.size())
            s += SIZE(*vtx.size());
        if (vtx.material())
            s += SIZE(*vtx.material());
        if (vtx.color())
            s += SIZE(*vtx.color());
        if (vtx.flag())
            s += SIZE(*vtx.flag());
        if (vtx.dup())
            s += SIZE(*vtx.dup());
        size += s * elms;
    }
    if (Int elms = edge.elms()) {
        UInt s = 0;
        if (edge.ind())
            s += SIZE(*edge.ind());
        if (edge.adjFace())
            s += SIZE(*edge.adjFace());
        if (edge.nrm())
            s += SIZE(*edge.nrm());
        if (edge.flag())
            s += SIZE(*edge.flag());
        if (edge.id())
            s += SIZE(*edge.id());
        size += s * elms;
    }
    if (Int elms = tri.elms()) {
        UInt s = 0;
        if (tri.ind())
            s += SIZE(*tri.ind());
        if (tri.adjFace())
            s += SIZE(*tri.adjFace());
        if (tri.adjEdge())
            s += SIZE(*tri.adjEdge());
        if (tri.nrm())
            s += SIZE(*tri.nrm());
        if (tri.flag())
            s += SIZE(*tri.flag());
        if (tri.id())
            s += SIZE(*tri.id());
        size += s * elms;
    }
    if (Int elms = quad.elms()) {
        UInt s = 0;
        if (quad.ind())
            s += SIZE(*quad.ind());
        if (quad.adjFace())
            s += SIZE(*quad.adjFace());
        if (quad.adjEdge())
            s += SIZE(*quad.adjEdge());
        if (quad.nrm())
            s += SIZE(*quad.nrm());
        if (quad.flag())
            s += SIZE(*quad.flag());
        if (quad.id())
            s += SIZE(*quad.id());
        size += s * elms;
    }
    return size;
}
MeshBase &MeshBase::include(MESH_FLAG f) {
    if (f & VTX_ALL) {
        Int elms = vtx.elms();
        if (f & VTX_POS && !vtx._pos)
            Alloc(vtx._pos, elms);
        if (f & VTX_NRM && !vtx._nrm)
            Alloc(vtx._nrm, elms);
        if (f & VTX_TAN && !vtx._tan)
            Alloc(vtx._tan, elms);
        if (f & VTX_BIN && !vtx._bin)
            Alloc(vtx._bin, elms);
        if (f & VTX_HLP && !vtx._hlp)
            Alloc(vtx._hlp, elms);
        if (f & VTX_TEX0 && !vtx._tex0)
            Alloc(vtx._tex0, elms);
        if (f & VTX_TEX1 && !vtx._tex1)
            Alloc(vtx._tex1, elms);
        if (f & VTX_TEX2 && !vtx._tex2)
            Alloc(vtx._tex2, elms);
        if (f & VTX_TEX3 && !vtx._tex3)
            Alloc(vtx._tex3, elms);
        if (f & VTX_MATRIX && !vtx._matrix)
            Alloc(vtx._matrix, elms);
        if (f & VTX_BLEND && !vtx._blend)
            Alloc(vtx._blend, elms);
        if (f & VTX_SIZE && !vtx._size)
            Alloc(vtx._size, elms);
        if (f & VTX_MATERIAL && !vtx._material)
            Alloc(vtx._material, elms);
        if (f & VTX_COLOR && !vtx._color)
            Alloc(vtx._color, elms);
        if (f & VTX_FLAG && !vtx._flag)
            Alloc(vtx._flag, elms);
        if (f & VTX_DUP && !vtx._dup)
            Alloc(vtx._dup, elms);
    }
    if (f & EDGE_ALL) {
        Int elms = edge.elms();
        if (f & EDGE_IND && !edge._ind)
            Alloc(edge._ind, elms);
        if (f & EDGE_ADJ_FACE && !edge._adj_face)
            Alloc(edge._adj_face, elms);
        if (f & EDGE_NRM && !edge._nrm)
            Alloc(edge._nrm, elms);
        if (f & EDGE_FLAG && !edge._flag)
            Alloc(edge._flag, elms);
        if (f & EDGE_ID && !edge._id)
            Alloc(edge._id, elms);
    }
    if (f & TRI_ALL) {
        Int elms = tri.elms();
        if (f & TRI_IND && !tri._ind)
            Alloc(tri._ind, elms);
        if (f & TRI_ADJ_FACE && !tri._adj_face)
            Alloc(tri._adj_face, elms);
        if (f & TRI_ADJ_EDGE && !tri._adj_edge)
            Alloc(tri._adj_edge, elms);
        if (f & TRI_NRM && !tri._nrm)
            Alloc(tri._nrm, elms);
        if (f & TRI_FLAG && !tri._flag)
            Alloc(tri._flag, elms);
        if (f & TRI_ID && !tri._id)
            Alloc(tri._id, elms);
    }
    if (f & QUAD_ALL) {
        Int elms = quad.elms();
        if (f & QUAD_IND && !quad._ind)
            Alloc(quad._ind, elms);
        if (f & QUAD_ADJ_FACE && !quad._adj_face)
            Alloc(quad._adj_face, elms);
        if (f & QUAD_ADJ_EDGE && !quad._adj_edge)
            Alloc(quad._adj_edge, elms);
        if (f & QUAD_NRM && !quad._nrm)
            Alloc(quad._nrm, elms);
        if (f & QUAD_FLAG && !quad._flag)
            Alloc(quad._flag, elms);
        if (f & QUAD_ID && !quad._id)
            Alloc(quad._id, elms);
    }
    return T;
}
MeshBase &MeshBase::exclude(MESH_FLAG f) {
    if (f & VTX_ALL) {
        if (f & VTX_POS)
            Free(vtx._pos);
        if (f & VTX_NRM)
            Free(vtx._nrm);
        if (f & VTX_TAN)
            Free(vtx._tan);
        if (f & VTX_BIN)
            Free(vtx._bin);
        if (f & VTX_HLP)
            Free(vtx._hlp);
        if (f & VTX_TEX0)
            Free(vtx._tex0);
        if (f & VTX_TEX1)
            Free(vtx._tex1);
        if (f & VTX_TEX2)
            Free(vtx._tex2);
        if (f & VTX_TEX3)
            Free(vtx._tex3);
        if (f & VTX_MATRIX)
            Free(vtx._matrix);
        if (f & VTX_BLEND)
            Free(vtx._blend);
        if (f & VTX_SIZE)
            Free(vtx._size);
        if (f & VTX_MATERIAL)
            Free(vtx._material);
        if (f & VTX_COLOR)
            Free(vtx._color);
        if (f & VTX_FLAG)
            Free(vtx._flag);
        if (f & VTX_DUP)
            Free(vtx._dup);
    }
    if (f & EDGE_ALL) {
        if (f & EDGE_IND)
            Free(edge._ind);
        if (f & EDGE_ADJ_FACE)
            Free(edge._adj_face);
        if (f & EDGE_NRM)
            Free(edge._nrm);
        if (f & EDGE_FLAG)
            Free(edge._flag);
        if (f & EDGE_ID)
            Free(edge._id);
    }
    if (f & TRI_ALL) {
        if (f & TRI_IND)
            Free(tri._ind);
        if (f & TRI_ADJ_FACE)
            Free(tri._adj_face);
        if (f & TRI_ADJ_EDGE)
            Free(tri._adj_edge);
        if (f & TRI_NRM)
            Free(tri._nrm);
        if (f & TRI_FLAG)
            Free(tri._flag);
        if (f & TRI_ID)
            Free(tri._id);
    }
    if (f & QUAD_ALL) {
        if (f & QUAD_IND)
            Free(quad._ind);
        if (f & QUAD_ADJ_FACE)
            Free(quad._adj_face);
        if (f & QUAD_ADJ_EDGE)
            Free(quad._adj_edge);
        if (f & QUAD_NRM)
            Free(quad._nrm);
        if (f & QUAD_FLAG)
            Free(quad._flag);
        if (f & QUAD_ID)
            Free(quad._id);
    }

    f = flag();
    if (!(f & VTX_ALL))
        vtx._elms = 0;
    if (!(f & EDGE_ALL))
        edge._elms = 0;
    if (!(f & TRI_ALL))
        tri._elms = 0;
    if (!(f & QUAD_ALL))
        quad._elms = 0;
    return T;
}
MeshBase &MeshBase::keepOnly(MESH_FLAG f) { return exclude(~f); }
/******************************************************************************/
MeshBase &MeshBase::del() {
    return keepOnly(MESH_NONE);
}
MeshBase &MeshBase::create(Int vtxs, Int edges, Int tris, Int quads, MESH_FLAG flag) {
    del();

    vtx._elms = vtxs;
    edge._elms = edges;
    tri._elms = tris;
    quad._elms = quads;
    include(flag | VTX_POS | EDGE_IND | FACE_IND);

    return T;
}
/******************************************************************************/
T1(TYPE)
static void Set(C Byte *&v, Int i, TYPE *data, MESH_FLAG flag) {
    if (data)
        data[i] = *(TYPE *)v;
    if (flag)
        v += SIZE(TYPE);
}

static INLINE void BoneSet(C Byte *&v, Int i, VtxBone *data, MESH_FLAG flag) {
#if 1 // #MeshVtxBoneHW
    if (data)
        data[i] = *(VecB4 *)v;
    if (flag)
        v += SIZE(VecB4);
#else
    Set(v, i, data, flag);
#endif
}

Bool MeshBase::createVtx(C VtxBuf &vb, MESH_FLAG flag, UInt storage, /*MeshRender::BoneSplit *bone_split, Int bone_splits, */ MESH_FLAG flag_and) {
    exclude(VTX_ALL);

    if (!vb._vtx_num)
        return true;

    if (C Byte *v = vb.lockRead()) {
        vtx._elms = vb._vtx_num;
        include(VTX_ALL & flag & flag_and);
        if (storage & MSHR_COMPRESS) {
            FREPA(vtx) {
                Set(v, i, vtx.pos(), flag & VTX_POS);
                if (vtx.nrm())
                    vtx.nrm(i) = SByte4ToNrm(*(VecB4 *)v);
                if (flag & VTX_NRM)
                    v += SIZE(VecB4);
                if (vtx.tan() || vtx.bin())
                    SByte4ToTan(*(VecB4 *)v, vtx.tan() ? &vtx.tan(i) : null, vtx.bin() ? &vtx.bin(i) : null, vtx.nrm() ? &vtx.nrm(i) : null);
                if (flag & VTX_TAN_BIN)
                    v += SIZE(VecB4);
                Set(v, i, vtx.hlp(), flag & VTX_HLP);
                Set(v, i, vtx.tex0(), flag & VTX_TEX0);
                Set(v, i, vtx.tex1(), flag & VTX_TEX1);
                Set(v, i, vtx.tex2(), flag & VTX_TEX2);
                Set(v, i, vtx.tex3(), flag & VTX_TEX3);
                BoneSet(v, i, vtx.matrix(), flag & VTX_MATRIX);
                Set(v, i, vtx.blend(), flag & VTX_BLEND);
                Set(v, i, vtx.size(), flag & VTX_SIZE);
                Set(v, i, vtx.material(), flag & VTX_MATERIAL);
                Set(v, i, vtx.color(), flag & VTX_COLOR);
            }
        } else {
            FREPA(vtx) {
                Set(v, i, vtx.pos(), flag & VTX_POS);
                Set(v, i, vtx.nrm(), flag & VTX_NRM);
                Set(v, i, vtx.tan(), flag & VTX_TAN);
                Set(v, i, vtx.bin(), flag & VTX_BIN);
                Set(v, i, vtx.hlp(), flag & VTX_HLP);
                Set(v, i, vtx.tex0(), flag & VTX_TEX0);
                Set(v, i, vtx.tex1(), flag & VTX_TEX1);
                Set(v, i, vtx.tex2(), flag & VTX_TEX2);
                Set(v, i, vtx.tex3(), flag & VTX_TEX3);
                BoneSet(v, i, vtx.matrix(), flag & VTX_MATRIX);
                Set(v, i, vtx.blend(), flag & VTX_BLEND);
                Set(v, i, vtx.size(), flag & VTX_SIZE);
                Set(v, i, vtx.material(), flag & VTX_MATERIAL);
                Set(v, i, vtx.color(), flag & VTX_COLOR);
            }
        }
        vb.unlock();

        /*// restore real bone matrix indexes
        if((storage&MSHR_BONE_SPLIT) && bone_split)if(VtxBone *matrix=vtx.matrix())FREP(bone_splits)
        {
         C MeshRender::BoneSplit &split=bone_split[i];
           FREP(split.vtxs)
           {
              matrix->x=split.split_to_real[matrix->x];
              matrix->y=split.split_to_real[matrix->y];
              matrix->z=split.split_to_real[matrix->z];
              matrix->w=split.split_to_real[matrix->w];
              matrix++;
           }
        }*/
        return true;
    }
    return false;
}
/******************************************************************************/
Bool MeshBase::createInd(C IndBuf &ib) {
    exclude(FACE_ALL | ADJ_ALL);

    if (ib._ind_num / 3 <= 0)
        return true;

    if (CPtr ind = ib.lockRead()) {
        tri._elms = ib._ind_num / 3;
        include(TRI_IND);

        if (ib.bit16())
            Copy16To32(tri.ind(), ind, tri.elms() * 3);
        else
            Copy32To32(tri.ind(), ind, tri.elms() * 3);

        ib.unlock();
        return true;
    }
    return false;
}
/******************************************************************************/
MeshBase &MeshBase::create(C MeshRender &src, MESH_FLAG flag_and) {
    del();
    if (createVtx(src._vb, src.flag(), src._storage, flag_and))
        createInd(src._ib);
    return T;
}
/******************************************************************************/
MeshBase &MeshBase::createPhys(C MeshLod &src, MESH_FLAG flag_and, Bool set_face_id_from_part_index, Bool skip_hidden_parts) {
    UInt part_flag = MSHP_NO_PHYS_BODY;
    if (skip_hidden_parts)
        part_flag |= MSHP_HIDDEN;
    REPA(src)
    if (src.parts[i].part_flag & part_flag) // if at least one needs to be removed
    {
        Memt<MeshBase> meshes;
        REPAD(p, src)
        if (!(src.parts[p].part_flag & part_flag)) // copy all desired MeshBase into container
        {
            MeshBase &mesh = meshes.New();
            mesh.create(src.parts[p], flag_and);
            if (set_face_id_from_part_index) {
                mesh.include(FACE_ID);
                REPA(mesh.tri)
                mesh.tri.id(i) = p;
                REPA(mesh.quad)
                mesh.quad.id(i) = p;
            }
        }
        create(meshes.data(), meshes.elms()); // create basing on the container
        return T;
    }
    create(src, flag_and, set_face_id_from_part_index); // create basing on the whole 'MeshLod'
    return T;
}
/******************************************************************************/
#if PHYSX
Bool MeshBase::create(PxConvexMesh &convex) {
    Int tris = 0;
    PxHullPolygon poly;
    const PxU8 *ind = convex.getIndexBuffer();

    // calculate number of tris
    REP(convex.getNbPolygons())
    if (convex.getPolygonData(i, poly))
        tris += Max(0, poly.mNbVerts - 2);

    // create
    create(convex.getNbVertices(), 0, tris, 0);

    // copy vertexes
    CopyN(vtx.pos(), (Vec *)convex.getVertices(), vtxs());

    // setup tris
    tris = 0;
    FREP(convex.getNbPolygons())
    if (convex.getPolygonData(i, poly))
        FREP(poly.mNbVerts - 2)
    tri.ind(tris++).set(ind[poly.mIndexBase + 0], ind[poly.mIndexBase + i + 1], ind[poly.mIndexBase + i + 2]);

    return true;
}
Bool MeshBase::create(PxTriangleMesh &mesh) {
    create(mesh.getNbVertices(), 0, mesh.getNbTriangles(), 0);
    CopyN(vtx.pos(), (Vec *)mesh.getVertices(), vtxs());
    if (mesh.getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES)
        Copy16To32(tri.ind(), mesh.getTriangles(), tris() * 3);
    else
        CopyN(tri.ind(), (VecI *)mesh.getTriangles(), tris());
    return true;
}
#else
Bool MeshBase::create(btConvexHullShape &convex) {
    Memt<Vec> points;
    points.setNum(convex.getNumPoints());
    REPA(points)
    points[i] = Bullet.vec(convex.getScaledPoint(i));
    createConvex(points.data(), points.elms());
    return true;
}
Bool MeshBase::create(btBvhTriangleMeshShape &mesh) {
    Bool ok = false;
    if (btStridingMeshInterface *smi = mesh.getMeshInterface()) {
        Vec *pos;
        VecI *ind;
        int vtxs, tris, vtx_stride, ind_stride;
        PHY_ScalarType vtx_type, ind_type;
        smi->getLockedReadOnlyVertexIndexBase((const unsigned char **)&pos, vtxs, vtx_type, vtx_stride, (const unsigned char **)&ind, ind_stride, tris, ind_type);
        if (vtx_type == PHY_FLOAT && ind_type == PHY_INTEGER) {
            create(vtxs, 0, tris, 0);
            CopyN(vtx.pos(), pos, vtxs);
            CopyN(tri.ind(), ind, tris);
            ok = true;
        }
        smi->unLockReadOnlyVertexBase(0);
    }
    if (!ok)
        del();
    return ok;
}
#endif
MeshBase &MeshBase::create(C PhysPart &part) {
    switch (part.type()) {
    case PHYS_SHAPE:
        return create(part.shape, MESH_NONE, (part.shape.type == SHAPE_BALL) ? 16 : ShapeTypeRound(part.shape.type) ? 32
                                                                                                                    : -1);

    case PHYS_CONVEX:
    case PHYS_MESH:
        if (part._pm) {
            if (part._pm->_base) {
                create(*part._pm->_base);
                return T;
            }
            if (part._pm->_convex) {
                create(*part._pm->_convex);
                return T;
            }
            if (part._pm->_mesh) {
                create(*part._pm->_mesh);
                return T;
            }
        }
        break;
    }
    return del();
}
/******************************************************************************/
MeshBase &MeshBase::create(C MeshBase *src[], Int elms, MESH_FLAG flag_and, Bool set_face_id_from_part_index) {
    if (!src)
        elms = 0;

    MESH_FLAG flag = MESH_NONE;
    Int vtxs = 0, edges = 0, tris = 0, quads = 0;
    MeshBase temp;

    REP(elms)
    if (C MeshBase *mesh = src[i]) {
        flag |= mesh->flag();
        vtxs += mesh->vtxs();
        edges += mesh->edges();
        tris += mesh->tris();
        quads += mesh->quads();
    }

    flag &= flag_and & (~(VTX_DUP | ADJ_ALL));
    if (set_face_id_from_part_index)
        flag |= ID_ALL;
    temp.create(vtxs, edges, tris, quads, flag);

    Vec *vtx_pos = temp.vtx.pos();
    Vec *vtx_nrm = temp.vtx.nrm();
    Vec *vtx_tan = temp.vtx.tan();
    Vec *vtx_bin = temp.vtx.bin();
    Vec *vtx_hlp = temp.vtx.hlp();
    Vec2 *vtx_tex0 = temp.vtx.tex0();
    Vec2 *vtx_tex1 = temp.vtx.tex1();
    Vec2 *vtx_tex2 = temp.vtx.tex2();
    Vec2 *vtx_tex3 = temp.vtx.tex3();
    VtxBone *vtx_matrix = temp.vtx.matrix();
    VecB4 *vtx_blend = temp.vtx.blend();
    Flt *vtx_size = temp.vtx.size();
    VecB4 *vtx_material = temp.vtx.material();
    Color *vtx_color = temp.vtx.color();
    Byte *vtx_flag = temp.vtx.flag();

    VecI2 *edge_ind = temp.edge.ind();
    Vec *edge_nrm = temp.edge.nrm();
    Byte *edge_flag = temp.edge.flag();
    Int *edge_id = temp.edge.id();

    VecI *tri_ind = temp.tri.ind();
    Vec *tri_nrm = temp.tri.nrm();
    Byte *tri_flag = temp.tri.flag();
    Int *tri_id = temp.tri.id();

    VecI4 *quad_ind = temp.quad.ind();
    Vec *quad_nrm = temp.quad.nrm();
    Byte *quad_flag = temp.quad.flag();
    Int *quad_id = temp.quad.id();

    // vertexes
    FREP(elms)
    if (C MeshBase *mesh = src[i]) if (Int vtxs = mesh->vtxs()) {
        if (vtx_pos) {
            CopyN(vtx_pos, mesh->vtx.pos(), vtxs);
            vtx_pos += vtxs;
        }
        if (vtx_nrm) {
            CopyN(vtx_nrm, mesh->vtx.nrm(), vtxs);
            vtx_nrm += vtxs;
        }
        if (vtx_tan) {
            CopyN(vtx_tan, mesh->vtx.tan(), vtxs);
            vtx_tan += vtxs;
        }
        if (vtx_bin) {
            CopyN(vtx_bin, mesh->vtx.bin(), vtxs);
            vtx_bin += vtxs;
        }
        if (vtx_hlp) {
            CopyN(vtx_hlp, mesh->vtx.hlp(), vtxs);
            vtx_hlp += vtxs;
        }
        if (vtx_tex0) {
            CopyN(vtx_tex0, mesh->vtx.tex0(), vtxs);
            vtx_tex0 += vtxs;
        }
        if (vtx_tex1) {
            CopyN(vtx_tex1, mesh->vtx.tex1(), vtxs);
            vtx_tex1 += vtxs;
        }
        if (vtx_tex2) {
            CopyN(vtx_tex2, mesh->vtx.tex2(), vtxs);
            vtx_tex2 += vtxs;
        }
        if (vtx_tex3) {
            CopyN(vtx_tex3, mesh->vtx.tex3(), vtxs);
            vtx_tex3 += vtxs;
        }
        if (vtx_matrix) {
            CopyN(vtx_matrix, mesh->vtx.matrix(), vtxs);
            vtx_matrix += vtxs;
        }
        if (vtx_size) {
            CopyN(vtx_size, mesh->vtx.size(), vtxs);
            vtx_size += vtxs;
        }
        if (vtx_flag) {
            CopyN(vtx_flag, mesh->vtx.flag(), vtxs);
            vtx_flag += vtxs;
        }
        if (vtx_blend) {
            if (mesh->vtx.blend())
                CopyN(vtx_blend, mesh->vtx.blend(), vtxs);
            else
                REP(vtxs)
                vtx_blend[i].set(255, 0, 0, 0);
            vtx_blend += vtxs;
        }
        if (vtx_material) {
            if (mesh->vtx.material())
                CopyN(vtx_material, mesh->vtx.material(), vtxs);
            else
                REP(vtxs)
                vtx_material[i].set(255, 0, 0, 0);
            vtx_material += vtxs;
        }
        if (vtx_color) {
            if (mesh->vtx.color())
                CopyN(vtx_color, mesh->vtx.color(), vtxs);
            else
                REP(vtxs)
                vtx_color[i] = WHITE;
            vtx_color += vtxs;
        }
    }

    // edges, tris, quads
    if (elms == 1) {
        if (C MeshBase *mesh = src[0]) {
            if (edge_ind)
                CopyN(edge_ind, mesh->edge.ind(), edges);
            if (edge_nrm)
                CopyN(edge_nrm, mesh->edge.nrm(), edges);
            if (edge_flag)
                CopyN(edge_flag, mesh->edge.flag(), edges);

            if (tri_ind)
                CopyN(tri_ind, mesh->tri.ind(), tris);
            if (tri_nrm)
                CopyN(tri_nrm, mesh->tri.nrm(), tris);
            if (tri_flag)
                CopyN(tri_flag, mesh->tri.flag(), tris);

            if (quad_ind)
                CopyN(quad_ind, mesh->quad.ind(), quads);
            if (quad_nrm)
                CopyN(quad_nrm, mesh->quad.nrm(), quads);
            if (quad_flag)
                CopyN(quad_flag, mesh->quad.flag(), quads);

            if (set_face_id_from_part_index) {
                REP(edges)
                edge_id[i] = 0;
                REP(tris)
                tri_id[i] = 0;
                REP(quads)
                quad_id[i] = 0;
            } else {
                if (edge_id)
                    CopyN(edge_id, mesh->edge.id(), edges);
                if (tri_id)
                    CopyN(tri_id, mesh->tri.id(), tris);
                if (quad_id)
                    CopyN(quad_id, mesh->quad.id(), quads);
            }
        }
    } else {
        Int vtx_ofs = 0;
        FREPD(m, elms)
        if (C MeshBase *mesh = src[m]) {
            if (Int edges = mesh->edges()) {
                if (edge_ind) {
                    C VecI2 *ind = mesh->edge.ind();
                    if (!ind) {
                        ZeroFastN(edge_ind, edges);
                        edge_ind += edges;
                    } else
                        REP(edges) *edge_ind++ = (*ind++) + vtx_ofs;
                }
                if (edge_nrm) {
                    CopyN(edge_nrm, mesh->edge.nrm(), edges);
                    edge_nrm += edges;
                }
                if (edge_flag) {
                    CopyN(edge_flag, mesh->edge.flag(), edges);
                    edge_flag += edges;
                }
                if (set_face_id_from_part_index)
                    REP(edges) *edge_id++ = m;
                else if (edge_id) {
                    CopyN(edge_id, mesh->edge.id(), edges);
                    edge_id += edges;
                }
            }
            if (Int tris = mesh->tris()) {
                if (tri_ind) {
                    C VecI *ind = mesh->tri.ind();
                    if (!ind) {
                        ZeroFastN(tri_ind, tris);
                        tri_ind += tris;
                    } else
                        REP(tris) *tri_ind++ = (*ind++) + vtx_ofs;
                }
                if (tri_nrm) {
                    CopyN(tri_nrm, mesh->tri.nrm(), tris);
                    tri_nrm += tris;
                }
                if (tri_flag) {
                    CopyN(tri_flag, mesh->tri.flag(), tris);
                    tri_flag += tris;
                }
                if (set_face_id_from_part_index)
                    REP(tris) *tri_id++ = m;
                else if (tri_id) {
                    CopyN(tri_id, mesh->tri.id(), tris);
                    tri_id += tris;
                }
            }
            if (Int quads = mesh->quads()) {
                if (quad_ind) {
                    C VecI4 *ind = mesh->quad.ind();
                    if (!ind) {
                        ZeroFastN(quad_ind, quads);
                        quad_ind += quads;
                    } else
                        REP(quads) *quad_ind++ = (*ind++) + vtx_ofs;
                }
                if (quad_nrm) {
                    CopyN(quad_nrm, mesh->quad.nrm(), quads);
                    quad_nrm += quads;
                }
                if (quad_flag) {
                    CopyN(quad_flag, mesh->quad.flag(), quads);
                    quad_flag += quads;
                }
                if (set_face_id_from_part_index)
                    REP(quads) *quad_id++ = m;
                else if (quad_id) {
                    CopyN(quad_id, mesh->quad.id(), quads);
                    quad_id += quads;
                }
            }
            vtx_ofs += mesh->vtxs();
        }
    }
    Swap(temp, T);
    return T;
}
MeshBase &MeshBase::create(C MeshBase *src, Int elms, MESH_FLAG flag_and, Bool set_face_id_from_part_index) {
    if (!src)
        elms = 0;
    Memt<C MeshBase *, 1024> mesh_ptr;
    mesh_ptr.setNum(elms);
    REPAO(mesh_ptr) = &src[i];
    return create(mesh_ptr.data(), mesh_ptr.elms(), flag_and, set_face_id_from_part_index);
}
MeshBase &MeshBase::create(C MeshBase &src, MESH_FLAG flag_and) {
    if (this == &src)
        keepOnly(flag_and);
    else {
        create(src.vtxs(), src.edges(), src.tris(), src.quads(), src.flag() & flag_and);
        copyVtxs(src);
        copyEdges(src);
        copyTris(src);
        copyQuads(src);
    }
    return T;
}
MeshBase &MeshBase::create(C MeshLod &src, MESH_FLAG flag_and, Bool set_face_id_from_part_index) {
    Memb<MeshBase> temp; // use 'Memb' because we're storing pointers to elms
    Memt<C MeshBase *, 1024> mesh_ptr;
    mesh_ptr.setNum(src.parts.elms());
    REPA(mesh_ptr) {
        C MeshPart &part = src.parts[i];
        if (part.render.is() && !part.base.is())
            mesh_ptr[i] = &temp.New().create(part.render, flag_and); // MeshBase needs to be created
        else
            mesh_ptr[i] = &part.base; // copy ptr of MeshLod's MeshBase
    }
    return create(mesh_ptr.data(), mesh_ptr.elms(), flag_and, set_face_id_from_part_index);
}
MeshBase &MeshBase::create(C MeshPart &src, MESH_FLAG flag_and) {
    if (src.base.is())
        return create(src.base, flag_and);
    if (src.render.is())
        return create(src.render, flag_and);
    return del();
}
MeshBase &MeshBase::copyFace(MeshBase &dest, C CMemPtr<Bool> &edge_is, C CMemPtr<Bool> &tri_is, C CMemPtr<Bool> &quad_is, MESH_FLAG flag_and) C {
    C Int *p;

    // vtx
    Memt<Bool> vtx_is;
    vtx_is.setNumZero(vtxs());
    Memt<Int> vtx_remap;
    if (edge_is)
        FREPA(edge)
        if (edge_is[i]) {
            p = edge.ind(i).c;
            REPD(j, 2)
            vtx_is[p[j]] = true;
        }
    if (tri_is)
        FREPA(tri)
        if (tri_is[i]) {
            p = tri.ind(i).c;
            REPD(j, 3)
            vtx_is[p[j]] = true;
        }
    if (quad_is)
        FREPA(quad)
        if (quad_is[i]) {
            p = quad.ind(i).c;
            REPD(j, 4)
            vtx_is[p[j]] = true;
        }

    // create copy
    MeshBase temp(CountIs(vtx_is),
                  CountIs(edge_is),
                  CountIs(tri_is),
                  CountIs(quad_is), flag() & flag_and & (~(VTX_DUP | ADJ_ALL)));
    temp.copyVtxs(T, vtx_is);
    temp.copyEdges(T, edge_is);
    temp.copyTris(T, tri_is);
    temp.copyQuads(T, quad_is);
    SetRemap(vtx_remap, vtx_is, vtxs());
    IndRemap(vtx_remap, temp.edge.ind(), temp.edges());
    IndRemap(vtx_remap, temp.tri.ind(), temp.tris());
    IndRemap(vtx_remap, temp.quad.ind(), temp.quads());

    Swap(dest, temp);
    return dest;
}
/******************************************************************************/
static void CopyID(MeshBase &dest, C MeshBase &src, Int id, Memt<Bool> &edge_is, Memt<Bool> &tri_is, Memt<Bool> &quad_is, MESH_FLAG flag_and) {
    edge_is.setNum(src.edges());
    tri_is.setNum(src.tris());
    quad_is.setNum(src.quads());

    // select
    C Int *_id;
    _id = src.edge.id();
    if (!_id)
        ZeroFastN(edge_is.data(), edge_is.elms());
    else
        FREPA(edge_is) {
            edge_is[i] = (*_id == id);
            _id++;
        }
    _id = src.tri.id();
    if (!_id)
        ZeroFastN(tri_is.data(), tri_is.elms());
    else
        FREPA(tri_is) {
            tri_is[i] = (*_id == id);
            _id++;
        }
    _id = src.quad.id();
    if (!_id)
        ZeroFastN(quad_is.data(), quad_is.elms());
    else
        FREPA(quad_is) {
            quad_is[i] = (*_id == id);
            _id++;
        }

    // copy
    src.copyFace(dest, edge_is, tri_is, quad_is, flag_and);

    /*// adjust edge flag
    if(Byte  *flag=dest.edge.flag())
    if(VecI2 *e_id=dest.edge.id  ())REPA(dest.edge)
    {
       *flag&=~ETQ_LR;
       if(e_id->x==id)*flag|=ETQ_R;
       if(e_id->y==id)*flag|=ETQ_L;
       flag++; e_id++;
    }*/
}
void MeshBase::copyID(MeshBase &dest, Int id, MESH_FLAG flag_and) C {
    Memt<Bool> edge_is, tri_is, quad_is;
    CopyID(dest, T, id, edge_is, tri_is, quad_is, flag_and);
}
void MeshBase::copyID(MeshLod &dest, MESH_FLAG flag_and) C {
    Memt<Bool> edge_is, tri_is, quad_is;
    dest.create(maxID() + 1);
    REPA(dest)
    CopyID(dest.parts[i].base, T, i, edge_is, tri_is, quad_is, flag_and);
}
void MeshBase::copyID(Mesh &dest, MESH_FLAG flag_and) C {
    dest.del();

    MeshLod &d = dest;
    copyID(d, flag_and);
    dest.setBox();
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
