/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************/
// GET
/******************************************************************************/
Int MeshBase::maxID() C {
    Int max = -1;
    if (edge.id())
        REPA(edge)
        MAX(max, edge.id(i));
    if (tri.id())
        REPA(tri)
        MAX(max, tri.id(i));
    if (quad.id())
        REPA(quad)
        MAX(max, quad.id(i));
    return max;
}
Bool MeshBase::hasID(Int id) C {
    if (edge.id())
        REPA(edge)
        if (edge.id(i) == id) return true;
    if (tri.id())
        REPA(tri)
        if (tri.id(i) == id) return true;
    if (quad.id())
        REPA(quad)
        if (quad.id(i) == id) return true;
    return false;
}
/******************************************************************************/
Bool MeshBase::getRect(Rect &rect) C {
    Box box;
    Bool ret = getBox(box);
    rect.min = box.min.xy;
    rect.max = box.max.xy;
    return ret;
}
Bool MeshBase::getRectXZ(Rect &rect) C {
    Box box;
    Bool ret = getBox(box);
    rect.min = box.min.xz();
    rect.max = box.max.xz();
    return ret;
}
Bool MeshBase::getBox(Box &box) C { return box.from(vtx.pos(), vtxs()); }
Bool MeshBase::getBox(Box &box, C Matrix3 &matrix) C { return box.from(vtx.pos(), vtxs(), matrix); }
Bool MeshBase::getBox(Box &box, C Matrix &matrix) C { return box.from(vtx.pos(), vtxs(), matrix); }
Bool MeshBase::getBall(Ball &ball) C { return ball.from(vtx.pos(), vtxs()); }

Flt MeshBase::area(Vec *center) C {
    if (center)
        center->zero();
    Flt area = 0;
    if (C Vec *vtx = T.vtx.pos()) {
        REPA(tri) {
            C VecI &f = tri.ind(i);
            Tri t(vtx[f.x], vtx[f.y], vtx[f.z]);
            Flt a = t.area();
            area += a;
            if (center)
                *center += a * t.center();
        }
        REPA(quad) {
            C VecI4 &f = quad.ind(i);
            Quad q(vtx[f.x], vtx[f.y], vtx[f.z], vtx[f.w]);
            Flt a = q.area();
            area += a;
            if (center)
                *center += a * q.center();
        }
    }
    if (center && area)
        *center /= area;
    return area;
}
/******************************************************************************/
Flt MeshBase::convexVolume() C {
    Flt vol = 0;
    Box box;
    if (getBox(box)) {
        C Vec *pos = vtx.pos(); // 'getBox' implies "vtx.pos()!=null"
        Vec center = box.center();
        REPA(tri) {
            C VecI &ind = tri.ind(i);
            vol += TetraVolume(pos[ind.x], pos[ind.y], pos[ind.z], center);
        }
        REPA(quad) {
            C VecI4 &ind = quad.ind(i);
            vol += TetraVolume(pos[ind.x], pos[ind.y], pos[ind.w], center);
            vol += TetraVolume(pos[ind.w], pos[ind.y], pos[ind.z], center);
        }
    }
    return vol;
}
/******************************************************************************/
// SET
/******************************************************************************/
MeshBase &MeshBase::setEdgeNormals(Bool flag) {
    exclude(FACE_NRM);
    include(EDGE_NRM);
    if (flag && !edge.flag())
        flag = false;
    REPA(edge) {
        Int *p = edge.ind(i).c;
        Int dir = 1;
        if (flag)
            switch (edge.flag(i) & ETQ_LR) {
            case 0:
            case ETQ_LR:
                dir = 0;
                break;
            case ETQ_L:
                dir = -1;
                break;
            }
        switch (dir) {
        case 0:
            edge.nrm(i).zero();
            break;
        case -1:
            edge.nrm(i).set(PerpN(vtx.pos(p[0]).xy - vtx.pos(p[1]).xy), 0);
            break;
        case +1:
            edge.nrm(i).set(PerpN(vtx.pos(p[1]).xy - vtx.pos(p[0]).xy), 0);
            break;
        }
    }
    return T;
}
MeshBase &MeshBase::setFaceNormals() {
    if (C Vec *pos = vtx.pos()) {
        exclude(EDGE_NRM);
        include(FACE_NRM);
        if (VecI *_tri = tri.ind()) {
            Vec *nrm = tri.nrm();
            REPA(tri) {
                Int *p = (_tri++)->c;
                *nrm++ = GetNormal(pos[p[0]], pos[p[1]], pos[p[2]]);
            }
        }
        if (VecI4 *_quad = quad.ind()) {
            Vec *nrm = quad.nrm();
            REPA(quad) {
                Int *p = (_quad++)->c;
                *nrm++ = GetNormal(pos[p[0]], pos[p[1]], pos[p[3]]);
            }
        }
    }
    return T;
}
MeshBase &MeshBase::setNormals2D(Bool flag) {
    setEdgeNormals(flag);
    include(VTX_NRM);
    ZeroN(vtx.nrm(), vtxs());

    VecI2 *_edge = edge.ind();
    Vec *nrm = edge.nrm();
    REPA(edge) {
        VecI2 f = *_edge++;
        f.remap(vtx.dup());
        vtx.nrm(f.c[0]) += *nrm;
        vtx.nrm(f.c[1]) += *nrm++;
    }
    Normalize(vtx.nrm(), vtxs());
    if (vtx.dup())
        REPA(vtx)
        vtx.nrm(i) = vtx.nrm(vtx.dup(i));
    return T;
}
MeshBase &MeshBase::setNormals() {
    include(VTX_NRM);
    ZeroN(vtx.nrm(), vtxs());

    REPA(tri) {
        VecI f = tri.ind(i);
        f.remap(vtx.dup());
        Vec nrm = GetNormalU(vtx.pos(f.c[0]), vtx.pos(f.c[1]), vtx.pos(f.c[2]));
        vtx.nrm(f.c[0]) += nrm;
        vtx.nrm(f.c[1]) += nrm;
        vtx.nrm(f.c[2]) += nrm;
    }

    REPA(quad) {
        VecI4 f = quad.ind(i);
        f.remap(vtx.dup());
        Vec &v0 = vtx.pos(f.c[0]),
            &v1 = vtx.pos(f.c[1]),
            &v2 = vtx.pos(f.c[2]),
            &v3 = vtx.pos(f.c[3]),
            nrm = GetNormalU(v0, v1, v3) + GetNormalU(v1, v2, v3);
        vtx.nrm(f.c[0]) += nrm;
        vtx.nrm(f.c[1]) += nrm;
        vtx.nrm(f.c[2]) += nrm;
        vtx.nrm(f.c[3]) += nrm;
    }
    /* Calculate using Quadrics (this is only for testing)
       Memt<QuadricMatrix> vtx_qm; vtx_qm.setNumZero(vtxs());
       REPA(tri)
       {
          VecI f     =tri.ind(i); f.remap(vtx.dup());
          Vec &v0    =vtx.pos(f.c[0]),
              &v1    =vtx.pos(f.c[1]),
              &v2    =vtx.pos(f.c[2]),
               nrm   =GetNormalU(v0, v1, v2);
          Flt  weight=nrm.normalize();
          QuadricMatrix qm(nrm, v0); qm*=weight;
          vtx_qm[f.c[0]]+=qm;
          vtx_qm[f.c[1]]+=qm;
          vtx_qm[f.c[2]]+=qm;
       }

       REPA(quad)
       {
          VecI4 f     =quad.ind(i); f.remap(vtx.dup());
          Vec  &v0    =vtx.pos(f.c[0]),
               &v1    =vtx.pos(f.c[1]),
               &v2    =vtx.pos(f.c[2]),
               &v3    =vtx.pos(f.c[3]),
                nrm013=GetNormalU(v0, v1, v3),
                nrm123=GetNormalU(v1, v2, v3);
          Flt   weight013=nrm013.normalize(),
                weight123=nrm123.normalize();
          QuadricMatrix qm013(nrm013, v0); qm013*=weight013;
          QuadricMatrix qm123(nrm123, v1); qm123*=weight123;
          vtx_qm[f.c[0]]+=qm013;
          vtx_qm[f.c[1]]+=qm013+qm123;
          vtx_qm[f.c[2]]+=      qm123;
          vtx_qm[f.c[3]]+=qm013+qm123;
       }

       REPA(vtx)vtx.nrm(i)=vtx_qm[i].normal(vtx.pos(i)+offset); // <- here we would have to use some offset because normal at the surface is not precise, best offset is vtx.nrm which we're trying to calculate
    */

    REPA(vtx) {
        Vec &nrm = vtx.nrm(i);
        if (!nrm.normalize())
            nrm.set(DEFAULT_VTX_NRM);
    } // !! valid non-zero normal must be set because otherwise NaN might get generated in the shader due to normalization of zero vectors !!
    if (vtx.dup())
        REPA(vtx)
        vtx.nrm(i) = vtx.nrm(vtx.dup(i));
    return T;
}
MeshBase &MeshBase::setNormalsAuto(Flt angle, Flt pos_eps) {
    explodeVtxs();
    include(VTX_NRM);
    ZeroN(vtx.nrm(), vtxs());

    {
        Memt<Vec> vtx_nrm_face;
        vtx_nrm_face.setNumZero(vtxs()); // vertex normals from face, unnormalized

        REPA(tri) {
            C VecI &f = tri.ind(i);
            Vec nrm = GetNormalU(vtx.pos(f.c[0]), vtx.pos(f.c[1]), vtx.pos(f.c[2]));
            vtx_nrm_face[f.c[0]] += nrm;
            vtx_nrm_face[f.c[1]] += nrm;
            vtx_nrm_face[f.c[2]] += nrm;
        }

        REPA(quad) {
            C VecI4 &f = quad.ind(i);
            C Vec &v0 = vtx.pos(f.c[0]),
                  &v1 = vtx.pos(f.c[1]),
                  &v2 = vtx.pos(f.c[2]),
                  &v3 = vtx.pos(f.c[3]),
                  nrm = GetNormalU(v0, v1, v3) + GetNormalU(v1, v2, v3);
            vtx_nrm_face[f.c[0]] += nrm;
            vtx_nrm_face[f.c[1]] += nrm;
            vtx_nrm_face[f.c[2]] += nrm;
            vtx_nrm_face[f.c[3]] += nrm;
        }

        Index vtx_vtx;
        linkVtxVtxSamePos(vtx_vtx, pos_eps, false); // link only single connections
        Flt cos = Cos(angle);
        REPA(vtx_vtx) // iterate all vtxs
        {
            C Vec &vtx_nrm = vtx_nrm_face[i];
            C IndexGroup &ig = vtx_vtx.group[i];
            REPAD(j, ig) // iterate all vtxs in the same position
            {
                Int test_vtx = ig[j];
                C Vec &test_vtx_nrm = vtx_nrm_face[test_vtx];
                if (CosBetween(vtx_nrm, test_vtx_nrm) >= cos) // use 'CosBetween' because vectors are not normalized, and >= instead of <= because we're comparing Cos instead of angles
                {                                             // share normals between vertexes
                    vtx.nrm(i) += test_vtx_nrm;
                    vtx.nrm(test_vtx) += vtx_nrm;
                }
            }
            vtx.nrm(i) += vtx_nrm; // add self
        }
    } // <- releases temp memory

    REPA(vtx) {
        Vec &nrm = vtx.nrm(i);
        if (!nrm.normalize())
            nrm.set(DEFAULT_VTX_NRM);
    } // !! valid non-zero normal must be set because otherwise NaN might get generated in the shader due to normalization of zero vectors !!

    return T;
}
/******************************************************************************/
MeshBase &MeshBase::setTanBin() {
    Bool set_tan = true, set_bin = true;

    if (set_tan || set_bin)               // want to set anything
        if (C Vec *pos = vtx.pos())       // we can calculate only if we have positions
            if (C Vec2 *tex = vtx.tex0()) // and tex coords
            {
                // when calculating tangents/binormals, we can't use any duplicates, because they have no knowledge about mirrored triangles, to detect them, tangents/binormals are needed, so first we set tan/bin per triangle, then we detect which go in same way, and then we merge
                include((set_tan ? VTX_TAN : MESH_NONE) | (set_bin ? VTX_BIN : MESH_NONE));
                if (set_tan)
                    ZeroN(vtx.tan(), vtxs());
                if (set_bin)
                    ZeroN(vtx.bin(), vtxs());

                if (VecI *_tri = tri.ind())
                    REPA(tri) {
                        VecI f = *_tri++; // here can't remap duplicates, because doing so would break mirrored triangles/texcoords
                        C Vec2 &t0 = tex[f.x],
                               ta = tex[f.y] - t0,
                               tb = tex[f.z] - t0;
                        C Vec &p0 = pos[f.x],
                              &p1 = pos[f.y],
                              &p2 = pos[f.z],
                              p01 = p1 - p0,
                              p02 = p2 - p0;
                        // Flt Tri::area()C {return 0.5f*Cross(p[1]-p[0], p[2]-p[0]).length();}
                        Flt length = Cross(p01, p02).length();
                        Flt u, v;
                        // tangent
                        // u*ta   + v*tb   = Vec2(1, 0)
                        // u*ta.x + v*tb.x = 1
                        // u*ta.y + v*tb.y = 0
                        if (set_tan && Solve(ta.x, ta.y, tb.x, tb.y, 1, 0, u, v) == 1) {
                            Vec tan = p01 * u + p02 * v;
                            tan.setLength(length); // make proportional to face area
                            vtx.tan(f.x) += tan;
                            vtx.tan(f.y) += tan;
                            vtx.tan(f.z) += tan;
                        }
                        // binormal
                        // u*ta   + v*tb   = Vec2(0, 1)
                        // u*ta.x + v*tb.x = 0
                        // u*ta.y + v*tb.y = 1
                        if (set_bin && Solve(ta.x, ta.y, tb.x, tb.y, 0, 1, u, v) == 1) {
                            Vec bin = p01 * u + p02 * v;
                            bin.setLength(length); // make proportional to face area
                            vtx.bin(f.x) += bin;
                            vtx.bin(f.y) += bin;
                            vtx.bin(f.z) += bin;
                        }
                    }
                if (VecI4 *_quad = quad.ind())
                    REPA(quad) {
                        VecI4 f = *_quad++; // here can't remap duplicates, because doing so would break mirrored triangles/texcoords
                        C Vec2 &t0 = tex[f.x],
                               ta = tex[f.y] - t0,
                               tb = tex[f.w] - t0;
                        C Vec &p0 = pos[f.x],
                              &p1 = pos[f.y],
                              &p2 = pos[f.z],
                              &p3 = pos[f.w],
                              p01 = p1 - p0,
                              p03 = p3 - p0,
                              p12 = p2 - p1,
                              p13 = p3 - p1;
                        // Flt Quad::area()C {return 0.5f*(Cross(p[1]-p[0], p[3]-p[0]).length()+Cross(p[2]-p[1], p[3]-p[1]).length());}
                        Flt length = Cross(p01, p03).length() + Cross(p12, p13).length();
                        Flt u, v;
                        if (set_tan && Solve(ta.x, ta.y, tb.x, tb.y, 1, 0, u, v) == 1) {
                            Vec tan = p01 * u + p03 * v;
                            tan.setLength(length); // make proportional to face area
                            vtx.tan(f.x) += tan;
                            vtx.tan(f.y) += tan;
                            vtx.tan(f.z) += tan;
                            vtx.tan(f.w) += tan;
                        }
                        if (set_bin && Solve(ta.x, ta.y, tb.x, tb.y, 0, 1, u, v) == 1) {
                            Vec bin = p01 * u + p03 * v;
                            bin.setLength(length); // make proportional to face area
                            vtx.bin(f.x) += bin;
                            vtx.bin(f.y) += bin;
                            vtx.bin(f.z) += bin;
                            vtx.bin(f.w) += bin;
                        }
                    }

                // merge neighbors
                // !! Warning: here vtx.tan, vtx.bin lengths are proportional to their face areas, and aren't normalized, so 'setVtxDupEx' 'Dot' tests for those vectors will work correctly only with 0 as eps cos
                setVtxDupEx(VTX_POS | VTX_NRM_TAN_BIN | VTX_TEX0, EPSD, EPS_COL_COS, 0, 0, true); // use small pos epsilon in case mesh is scaled down, use 0 eps cos for tan/bin to only test if they're on the same side (also only this value can work for unnormalized vectors), for tan/bin we can use tex_wrap=true
                if (vtx.dup())
                    REPA(vtx) {
                        Int dup = vtx.dup(i);
                        if (dup != i) // duplicate
                        {             // add to unique
                            if (set_tan)
                                vtx.tan(dup) += vtx.tan(i);
                            if (set_bin)
                                vtx.bin(dup) += vtx.bin(i);
                        }
                    }

                // normalize unique
                REPA(vtx) {
                    if (vtx.dup() && vtx.dup(i) != i)
                        continue; // skip for duplicates
                    if (set_tan) {
                        Vec &tan = vtx.tan(i);
                        if (vtx.nrm()) // if have normals
                        {
                            C Vec &nrm = vtx.nrm(i);
                            tan = PointOnPlane(tan, nrm);         // put on normal plane, this is needed for some models that have smooth vtx normals, but achieve flat surfaces due to normal maps. This is also needed to make tan perpendicular to nrm, so valid non-zero 'bin' can be generated out of the pair (in order to avoid zero which might generate NaN in the shader)
                            if (!tan.normalize()                  // if tan is zero
                                || Abs(Dot(nrm, tan)) >= SQRT2_2) // or after normalization it's close to normal (this can happen due to numerical precision issues)
                                tan = PerpN(nrm);                 // set as any perpendicular vector to normal
                        } else if (!tan.normalize())
                            tan.set(DEFAULT_VTX_TAN); // !! valid non-zero tangent must be set because otherwise NaN might get generated in the shader due to normalization of zero vectors !!
                    }
                    if (set_bin) {
                        Vec &bin = vtx.bin(i);
                        if (!bin.normalize()) // !! valid non-zero binormal must be set because otherwise NaN might get generated in the shader due to normalization of zero vectors !!
                        {
                            if (vtx.nrm() && vtx.tan())
                                bin = CrossN(vtx.nrm(i), vtx.tan(i));
                            else if (vtx.nrm())
                                bin = PerpN(vtx.nrm(i));
                            else if (vtx.tan())
                                bin = PerpN(vtx.tan(i));
                            else
                                bin.set(DEFAULT_VTX_BIN);
                        }
                    }
                }

                // set duplicates from unique
                if (vtx.dup()) {
                    REPA(vtx) {
                        Int dup = vtx.dup(i);
                        if (dup != i) // duplicate
                        {
                            if (set_tan)
                                vtx.tan(i) = vtx.tan(dup);
                            if (set_bin)
                                vtx.bin(i) = vtx.bin(dup);
                        }
                    }
                    exclude(VTX_DUP);
                }
            }
    return T;
}
MeshBase &MeshBase::setTanBinDbl() {
    Bool set_tan = true, set_bin = true;

    if (set_tan || set_bin)               // want to set anything
        if (C Vec *pos = vtx.pos())       // we can calculate only if we have positions
            if (C Vec2 *tex = vtx.tex0()) // and tex coords
            {
                // when calculating tangents/binormals, we can't use any duplicates, because they have no knowledge about mirrored triangles, to detect them, tangents/binormals are needed, so first we set tan/bin per triangle, then we detect which go in same way, and then we merge
                include((set_tan ? VTX_TAN : MESH_NONE) | (set_bin ? VTX_BIN : MESH_NONE));
                Mems<VecD> vtx_tan, vtx_bin;
                if (set_tan)
                    vtx_tan.setNumZero(vtxs());
                if (set_bin)
                    vtx_bin.setNumZero(vtxs());

                if (VecI *_tri = tri.ind())
                    REPA(tri) {
                        VecI f = *_tri++; // here can't remap duplicates, because doing so would break mirrored triangles/texcoords
                        C Vec2 &t0 = tex[f.x],
                               ta = tex[f.y] - t0,
                               tb = tex[f.z] - t0;
                        C Vec &p0 = pos[f.x],
                              &p1 = pos[f.y],
                              &p2 = pos[f.z];
                        C VecD p01 = p1 - p0,
                               p02 = p2 - p0;
                        // Dbl Tri::area()C {return 0.5*Cross(p[1]-p[0], p[2]-p[0]).length();}
                        Dbl length = Cross(p01, p02).length();
                        Dbl u, v;
                        // tangent
                        // u*ta   + v*tb   = Vec2(1, 0)
                        // u*ta.x + v*tb.x = 1
                        // u*ta.y + v*tb.y = 0
                        if (set_tan && Solve(ta.x, ta.y, tb.x, tb.y, 1, 0, u, v) == 1) {
                            VecD tan = p01 * u + p02 * v;
                            tan.setLength(length); // make proportional to face area
                            vtx_tan[f.x] += tan;
                            vtx_tan[f.y] += tan;
                            vtx_tan[f.z] += tan;
                        }
                        // binormal
                        // u*ta   + v*tb   = Vec2(0, 1)
                        // u*ta.x + v*tb.x = 0
                        // u*ta.y + v*tb.y = 1
                        if (set_bin && Solve(ta.x, ta.y, tb.x, tb.y, 0, 1, u, v) == 1) {
                            VecD bin = p01 * u + p02 * v;
                            bin.setLength(length); // make proportional to face area
                            vtx_bin[f.x] += bin;
                            vtx_bin[f.y] += bin;
                            vtx_bin[f.z] += bin;
                        }
                    }
                if (VecI4 *_quad = quad.ind())
                    REPA(quad) {
                        VecI4 f = *_quad++; // here can't remap duplicates, because doing so would break mirrored triangles/texcoords
                        C Vec2 &t0 = tex[f.x],
                               ta = tex[f.y] - t0,
                               tb = tex[f.w] - t0;
                        C Vec &p0 = pos[f.x],
                              &p1 = pos[f.y],
                              &p2 = pos[f.z],
                              &p3 = pos[f.w];
                        C VecD p01 = p1 - p0,
                               p03 = p3 - p0,
                               p12 = p2 - p1,
                               p13 = p3 - p1;
                        // Dbl Quad::area()C {return 0.5*(Cross(p[1]-p[0], p[3]-p[0]).length()+Cross(p[2]-p[1], p[3]-p[1]).length());}
                        Dbl length = Cross(p01, p03).length() + Cross(p12, p13).length();
                        Dbl u, v;
                        if (set_tan && Solve(ta.x, ta.y, tb.x, tb.y, 1, 0, u, v) == 1) {
                            VecD tan = p01 * u + p03 * v;
                            tan.setLength(length); // make proportional to face area
                            vtx_tan[f.x] += tan;
                            vtx_tan[f.y] += tan;
                            vtx_tan[f.z] += tan;
                            vtx_tan[f.w] += tan;
                        }
                        if (set_bin && Solve(ta.x, ta.y, tb.x, tb.y, 0, 1, u, v) == 1) {
                            VecD bin = p01 * u + p03 * v;
                            bin.setLength(length); // make proportional to face area
                            vtx_bin[f.x] += bin;
                            vtx_bin[f.y] += bin;
                            vtx_bin[f.z] += bin;
                            vtx_bin[f.w] += bin;
                        }
                    }

                // merge neighbors
                // !! Warning: here vtx.tan, vtx.bin lengths are proportional to their face areas, and aren't normalized, so 'setVtxDupEx' 'Dot' tests for those vectors will work correctly only with 0 as eps cos
                if (set_tan)
                    REPA(vtx)
                    vtx.tan(i) = vtx_tan[i];
                if (set_bin)
                    REPA(vtx)
                    vtx.bin(i) = vtx_bin[i];
                setVtxDupEx(VTX_POS | VTX_NRM_TAN_BIN | VTX_TEX0, EPSD, EPS_COL_COS, 0, 0, true); // use small pos epsilon in case mesh is scaled down, use 0 eps cos for tan/bin to only test if they're on the same side (also only this value can work for unnormalized vectors), for tan/bin we can use tex_wrap=true
                if (vtx.dup())
                    REPA(vtx) {
                        Int dup = vtx.dup(i);
                        if (dup != i) // duplicate
                        {             // add to unique
                            if (set_tan)
                                vtx_tan[dup] += vtx_tan[i];
                            if (set_bin)
                                vtx_bin[dup] += vtx_bin[i];
                        }
                    }

                // normalize unique
                REPA(vtx) {
                    if (vtx.dup() && vtx.dup(i) != i)
                        continue; // skip for duplicates
                    if (set_tan) {
                        VecD &tan_src = vtx_tan[i];
                        Vec &tan_dst = vtx.tan(i);
                        if (vtx.nrm()) // if have normals
                        {
                            C Vec &nrm = vtx.nrm(i);
                            tan_src = PointOnPlane(tan_src, nrm);     // put on normal plane, this is needed for some models that have smooth vtx normals, but achieve flat surfaces due to normal maps. This is also needed to make tan perpendicular to nrm, so valid non-zero 'bin' can be generated out of the pair (in order to avoid zero which might generate NaN in the shader)
                            if (!tan_src.normalize()                  // if tan is zero
                                || Abs(Dot(nrm, tan_src)) >= SQRT2_2) // or after normalization it's close to normal (this can happen due to numerical precision issues)
                            {
                                tan_dst = PerpN(nrm);
                                goto tan_set;
                            } // set as any perpendicular vector to normal
                        } else if (!tan_src.normalize()) {
                            tan_dst.set(DEFAULT_VTX_TAN);
                            goto tan_set;
                        } // !! valid non-zero tangent must be set because otherwise NaN might get generated in the shader due to normalization of zero vectors !!
                        tan_dst = tan_src;
                    tan_set:;
                    }
                    if (set_bin) {
                        VecD &bin_src = vtx_bin[i];
                        Vec &bin_dst = vtx.bin(i);
                        if (bin_src.normalize())
                            bin_dst = bin_src;
                        else // !! valid non-zero binormal must be set because otherwise NaN might get generated in the shader due to normalization of zero vectors !!
                        {
                            if (vtx.nrm() && vtx.tan())
                                bin_dst = CrossN(vtx.nrm(i), vtx.tan(i));
                            else if (vtx.nrm())
                                bin_dst = PerpN(vtx.nrm(i));
                            else if (vtx.tan())
                                bin_dst = PerpN(vtx.tan(i));
                            else
                                bin_dst.set(DEFAULT_VTX_BIN);
                        }
                    }
                }

                // set duplicates from unique
                if (vtx.dup()) {
                    REPA(vtx) {
                        Int dup = vtx.dup(i);
                        if (dup != i) // duplicate
                        {
                            if (set_tan)
                                vtx.tan(i) = vtx.tan(dup);
                            if (set_bin)
                                vtx.bin(i) = vtx.bin(dup);
                        }
                    }
                    exclude(VTX_DUP);
                }
            }
    return T;
}
/******************************************************************************/
MeshBase &MeshBase::setAutoTanBin() {
    if (vtx.nrm()) // we will need tan/bin only if we have normals
    {
        setTanBin();
        // check if we can remove binormals if they can be reconstructed from Nrm Tan
#if 0 // skip this because it's used only for game mesh which always encodes TanBin packed together
      if(vtx.bin())
      {
         REPA(vtx)
         {
            Vec bin=Cross(vtx.nrm(i), vtx.tan(i));
            if(Dot(bin, vtx.bin(i))<here some eps_cos)return T; // if binormals are different, then it means that binormal is necessary, so keep it and return without deleting it
         }
         exclude(VTX_BIN); // binormal unnecessary
      }
#endif
    } else
        exclude(VTX_TAN_BIN);
    return T;
}
/******************************************************************************
MeshBase& setSolid(); // fill 'edge.flag' EDGE_FLAG with solid info

MeshBase& MeshBase::setSolid()
{
   if(!edge.adj_face)setAdjacencies();
   if(!edge.flag    )AllocZero     (edge.flag, edge.num);
   Byte  *flag=edge.flag;
   VecI2 *adj =edge.adj_face;
   REP(edge.num)
   {
                       *flag&=~ETQ_LR;
      if(adj->c[0]!=-1)*flag|= ETQ_R;
      if(adj->c[1]!=-1)*flag|= ETQ_L;
      flag++;
      adj ++;
   }
   return T;
}
/******************************************************************************
MeshBase& setID         (Int  solid, Int not_solid=-1                    ); // set face ID values (edge.id, tri.id, quad.id) to 'solid' / 'not_solid'
MeshBase& MeshBase::setID(Int solid, Int not_solid)
{
   include(ID_ALL);
   ;
   VecI2 *eid =edge.id,
         *tid =tri .id,
         *qid =quad.id;
   if(C Byte *flag=edge.flag)
   {
      REPA(edge)
      {
         (eid++)->set(((*flag)&ETQ_R) ? solid : not_solid,
                      ((*flag)&ETQ_L) ? solid : not_solid);
         flag++;
      }
   }else
   {
      REPA(edge)eid[i].set(not_solid);
   }
   REPA(tri )tid[i].set(solid, not_solid);
   REPA(quad)qid[i].set(solid, not_solid);
   return T;
}
/******************************************************************************/
struct MeshAO {
    static Flt Full(Flt x) { return 0; }
    static Flt Quartic(Flt x) { return Sqr(Sqr(x)); }
    static Flt Square(Flt x) { return Sqr(x); }
    static Flt Linear(Flt x) { return x; }
    static Flt LinearR(Flt x) { return 2 - 2 / (x + 1); }
    static Flt SquareR(Flt x) { return 1 - Sqr(1 - x); }

    C Vec *pos, *nrm;
    Color *col;
    Memt<Vec> ray_dir;
    Byte max;
    Flt mul, add, ray_length, pos_eps;
    PhysPart body;
    Flt (*func)(Flt x);

    INLINE Bool sweep(C Vec &pos, C Vec &move, Flt *frac) {
#if 0
      return Sweep(pos, move, *mesh, null, frac, null, null, true, two_sided);
#else
        PhysHitBasic hit;
        if (body.ray(pos, move, null, frac ? &hit : null, true)) {
            if (frac)
                *frac = hit.frac;
            return true;
        }
        return false;
#endif
    }
    static void Set(IntPtr elm_index, MeshAO &ao, Int thread_index) { ao.set(elm_index); }
    void set(Int vtx) {
        C Vec &pos = T.pos[vtx], &nrm = T.nrm[vtx];
        Dbl light = 0;
        REPA(ray_dir) {
            C Vec &ray = ray_dir[i];
            Flt d = Dot(nrm, ray);
            if (d > 0) {
                Flt frac;
                if (sweep(pos + ray * pos_eps, ray, &frac))
                    light += func(frac) * d;
                else
                    light += d;
            }
        }
        light = light * mul + add;
#if LINEAR_GAMMA
        Byte light_b = Max(LinearToByteSRGB(light), max);
#else
        Byte light_b = Mid(RoundPos(light * 255), max, 255);
#endif
        Color &c = col[vtx];
        c.r = c.g = c.b = light_b; // keep alpha
    }
    void process(Int vtxs, Threads *threads) {
        if (threads)
            threads->process1(vtxs, Set, T);
        else
            REP(vtxs)
            set(i);
    }
    MeshAO(Flt strength, Flt bias, Flt max, Flt ray_length, Flt pos_eps, Int rays, MESH_AO_FUNC func) {
        switch (func) {
        default:
        case MAF_FULL:
            T.func = Full;
            break;
        case MAF_QUARTIC:
            T.func = Quartic;
            break;
        case MAF_SQUARE:
            T.func = Square;
            break;
        case MAF_LINEAR:
            T.func = Linear;
            break;
        case MAF_LINEAR_REV:
            T.func = LinearR;
            break;
        case MAF_SQUARE_REV:
            T.func = SquareR;
            break;
        }
        T.ray_length = Max(0, ray_length);
        T.pos_eps = Mid(pos_eps, 0.0f, T.ray_length * 0.5f);
        T.ray_length -= T.pos_eps;
        if (T.ray_length)
            T.pos_eps /= T.ray_length; // because we pre-multiply 'ray_dir' with 'ray_length' then we need to adjust 'pos_eps'
        T.max = FltToByte(1 - max);

        Int rays_res = Max(1, Round(Sqrt(rays / 6.0f))); // rays resolution in 2D, in one of 6 cube faces
        rays = Sqr(rays_res) * 6;                        // total number of rays
        ray_dir.setNum(rays);
        Int r = 0, r_pos = 0;
        Flt ray_step = PI_2 / rays_res;
        Vec ray_base;
        ray_base.z = 1;
        Dbl ray_sum = 0;
        REPD(x, rays_res) {
            ray_base.x = Tan(x * ray_step - PI_4);
            REPD(y, rays_res) {
                ray_base.y = Tan(y * ray_step - PI_4);
                Vec n = ray_base;
                n.normalize(); // normalized
                REPD(f, 6)     // 6 cube faces
                {
                    Vec ray;
                    switch (f) {
                    case 0:
                        ray.set(n.x, n.y, n.z);
                        break;
                    case 1:
                        ray.set(-n.z, n.y, n.x);
                        break;
                    case 2:
                        ray.set(-n.x, -n.y, -n.z);
                        break;
                    case 3:
                        ray.set(n.z, -n.y, -n.x);
                        break;
                    case 4:
                        ray.set(n.x, n.z, -n.y);
                        break;
                    case 5:
                        ray.set(-n.x, -n.z, n.y);
                        break;
                    }
                    ray *= T.ray_length; // pre-multiply so we don't have to do this for each vertex*ray
                    ray_dir[r++] = ray;
                    if (ray.x > 0) {
                        r_pos++;
                        ray_sum += ray.x;
                    } // Dot(ray, Vec(1, 0, 0)), calculate total amount in one hemisphere (X+)
                }
            }
        }

        add = (1 - strength + strength * bias);
        mul = strength / ray_sum;
        // mul=   strength/r_pos  ; // this is for light independent on the Dot product
    }
};
/******************************************************************************/
MeshBase &MeshBase::setVtxAO(Flt strength, Flt bias, Flt max, Flt ray_length, Flt pos_eps, Int rays, MESH_AO_FUNC func, Threads *threads) {
    if (vtx.pos() && vtx.nrm()) {
        MeshAO ao(strength, bias, max, ray_length, pos_eps, rays, func);
        if (ao.body.createMeshTry(T)) {
            if (!vtx.color()) {
                include(VTX_COLOR);
                REPA(vtx)
                vtx.color(i) = WHITE;
            } // setup colors, especially needed for Alpha which is kept and not overwritten, but set all components in case some codes below don't get executed
            if (ao.pos = vtx.pos())
                if (ao.nrm = vtx.nrm())
                    if (ao.col = vtx.color())
                        ao.process(vtxs(), threads);
        }
    }
    return T;
}
MeshLod &MeshLod::setVtxAO(Flt strength, Flt bias, Flt max, Flt ray_length, Flt pos_eps, Int rays, MESH_AO_FUNC func, Threads *threads) {
    if (parts.elms()) {
        MeshAO ao(strength, bias, max, ray_length, pos_eps, rays, func);
        if (ao.body.createMeshTry(T, false, true))
            REPA(T) {
                MeshPart &part = parts[i];
                MeshBase &base = part.base;
                if (!base.vtx.color()) {
                    base.include(VTX_COLOR);
                    REPA(base.vtx)
                    base.vtx.color(i) = WHITE;
                } // setup colors, especially needed for Alpha which is kept and not overwritten, but set all components in case some codes below don't get executed
                if (ao.pos = base.vtx.pos())
                    if (ao.nrm = base.vtx.nrm())
                        if (ao.col = base.vtx.color())
                            ao.process(base.vtxs(), threads);
            }
    }
    return T;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
