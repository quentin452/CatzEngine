/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************

   Remember that 'QuadN.n' in many cases may be inaccurate,
      that is for cases when quad is built from 2 non co-planar triangles.

/******************************************************************************/
Quad ::Quad(C QuadD &quad) {
    p[0] = quad.p[0];
    p[1] = quad.p[1];
    p[2] = quad.p[2];
    p[3] = quad.p[3];
}
QuadN ::QuadN(C Quad &quad) {
    p[0] = quad.p[0];
    p[1] = quad.p[1];
    p[2] = quad.p[2];
    p[3] = quad.p[3];
    setNormal();
}
QuadN ::QuadN(C QuadND &quad) {
    p[0] = quad.p[0];
    p[1] = quad.p[1];
    p[2] = quad.p[2];
    p[3] = quad.p[3];
    n = quad.n;
}
QuadD ::QuadD(C Quad &quad) {
    p[0] = quad.p[0];
    p[1] = quad.p[1];
    p[2] = quad.p[2];
    p[3] = quad.p[3];
}
QuadND::QuadND(C QuadD &quad) {
    p[0] = quad.p[0];
    p[1] = quad.p[1];
    p[2] = quad.p[2];
    p[3] = quad.p[3];
    setNormal();
}
QuadND::QuadND(C Quad &quad) {
    p[0] = quad.p[0];
    p[1] = quad.p[1];
    p[2] = quad.p[2];
    p[3] = quad.p[3];
    setNormal();
} // recalculate for high precision instead of using 'quad.n'

Quad2 &Quad2 ::operator+=(C Vec2 &v) {
    p[0] += v;
    p[1] += v;
    p[2] += v;
    p[3] += v;
    return T;
}
Quad2 &Quad2 ::operator-=(C Vec2 &v) {
    p[0] -= v;
    p[1] -= v;
    p[2] -= v;
    p[3] -= v;
    return T;
}
QuadD2 &QuadD2::operator+=(C VecD2 &v) {
    p[0] += v;
    p[1] += v;
    p[2] += v;
    p[3] += v;
    return T;
}
QuadD2 &QuadD2::operator-=(C VecD2 &v) {
    p[0] -= v;
    p[1] -= v;
    p[2] -= v;
    p[3] -= v;
    return T;
}
Quad &Quad ::operator+=(C Vec &v) {
    p[0] += v;
    p[1] += v;
    p[2] += v;
    p[3] += v;
    return T;
}
Quad &Quad ::operator-=(C Vec &v) {
    p[0] -= v;
    p[1] -= v;
    p[2] -= v;
    p[3] -= v;
    return T;
}
QuadN &QuadN ::operator+=(C Vec &v) {
    p[0] += v;
    p[1] += v;
    p[2] += v;
    p[3] += v;
    return T;
}
QuadN &QuadN ::operator-=(C Vec &v) {
    p[0] -= v;
    p[1] -= v;
    p[2] -= v;
    p[3] -= v;
    return T;
}
QuadD &QuadD ::operator+=(C VecD &v) {
    p[0] += v;
    p[1] += v;
    p[2] += v;
    p[3] += v;
    return T;
}
QuadD &QuadD ::operator-=(C VecD &v) {
    p[0] -= v;
    p[1] -= v;
    p[2] -= v;
    p[3] -= v;
    return T;
}
QuadND &QuadND::operator+=(C VecD &v) {
    p[0] += v;
    p[1] += v;
    p[2] += v;
    p[3] += v;
    return T;
}
QuadND &QuadND::operator-=(C VecD &v) {
    p[0] -= v;
    p[1] -= v;
    p[2] -= v;
    p[3] -= v;
    return T;
}
Quad2 &Quad2 ::operator*=(Flt r) {
    p[0] *= r;
    p[1] *= r;
    p[2] *= r;
    p[3] *= r;
    return T;
}
Quad2 &Quad2 ::operator/=(Flt r) {
    p[0] /= r;
    p[1] /= r;
    p[2] /= r;
    p[3] /= r;
    return T;
}
QuadD2 &QuadD2::operator*=(Dbl r) {
    p[0] *= r;
    p[1] *= r;
    p[2] *= r;
    p[3] *= r;
    return T;
}
QuadD2 &QuadD2::operator/=(Dbl r) {
    p[0] /= r;
    p[1] /= r;
    p[2] /= r;
    p[3] /= r;
    return T;
}
Quad &Quad ::operator*=(Flt r) {
    p[0] *= r;
    p[1] *= r;
    p[2] *= r;
    p[3] *= r;
    return T;
}
Quad &Quad ::operator/=(Flt r) {
    p[0] /= r;
    p[1] /= r;
    p[2] /= r;
    p[3] /= r;
    return T;
}
QuadN &QuadN ::operator*=(Flt r) {
    p[0] *= r;
    p[1] *= r;
    p[2] *= r;
    p[3] *= r;
    return T;
}
QuadN &QuadN ::operator/=(Flt r) {
    p[0] /= r;
    p[1] /= r;
    p[2] /= r;
    p[3] /= r;
    return T;
}
QuadD &QuadD ::operator*=(Dbl r) {
    p[0] *= r;
    p[1] *= r;
    p[2] *= r;
    p[3] *= r;
    return T;
}
QuadD &QuadD ::operator/=(Dbl r) {
    p[0] /= r;
    p[1] /= r;
    p[2] /= r;
    p[3] /= r;
    return T;
}
QuadND &QuadND::operator*=(Dbl r) {
    p[0] *= r;
    p[1] *= r;
    p[2] *= r;
    p[3] *= r;
    return T;
}
QuadND &QuadND::operator/=(Dbl r) {
    p[0] /= r;
    p[1] /= r;
    p[2] /= r;
    p[3] /= r;
    return T;
}
/******************************************************************************/
Quad &Quad::set(C Vec &p0, C Vec &p1, C Vec &p2, C Vec &p3) {
    p[0] = p0;
    p[1] = p1;
    p[2] = p2;
    p[3] = p3;
    return T;
}
QuadN &QuadN::set(C Vec &p0, C Vec &p1, C Vec &p2, C Vec &p3, C Vec *normal) {
    p[0] = p0;
    p[1] = p1;
    p[2] = p2;
    p[3] = p3;
    if (normal)
        n = *normal;
    else
        setNormal();
    return T;
}
QuadD &QuadD::set(C VecD &p0, C VecD &p1, C VecD &p2, C VecD &p3) {
    p[0] = p0;
    p[1] = p1;
    p[2] = p2;
    p[3] = p3;
    return T;
}
QuadND &QuadND::set(C VecD &p0, C VecD &p1, C VecD &p2, C VecD &p3, C VecD *normal) {
    p[0] = p0;
    p[1] = p1;
    p[2] = p2;
    p[3] = p3;
    if (normal)
        n = *normal;
    else
        setNormal();
    return T;
}
/******************************************************************************/
Flt QuadArea2(C Vec2 &p0, C Vec2 &p1, C Vec2 &p2, C Vec2 &p3) {
    auto e0 = p1 - p0, e1 = p3 - p1, e2 = p1 - p2;
    return Abs(Cross(e0, e1)) + Abs(Cross(e1, e2));
} // use 'Abs' 2 times to support concave quads ("Abs(Cross+Cross)" supports only convex)
Dbl QuadArea2(C VecD2 &p0, C VecD2 &p1, C VecD2 &p2, C VecD2 &p3) {
    auto e0 = p1 - p0, e1 = p3 - p1, e2 = p1 - p2;
    return Abs(Cross(e0, e1)) + Abs(Cross(e1, e2));
} // use 'Abs' 2 times to support concave quads ("Abs(Cross+Cross)" supports only convex)

Flt QuadArea2(C Vec &p0, C Vec &p1, C Vec &p2, C Vec &p3) {
    auto e0 = p1 - p0, e1 = p3 - p1, e2 = p1 - p2;
    return Cross(e0, e1).length() + Cross(e1, e2).length();
}
Dbl QuadArea2(C VecD &p0, C VecD &p1, C VecD &p2, C VecD &p3) {
    auto e0 = p1 - p0, e1 = p3 - p1, e2 = p1 - p2;
    return Cross(e0, e1).length() + Cross(e1, e2).length();
}

Flt Quad2 ::area() C { return 0.5f * QuadArea2(p[0], p[1], p[2], p[3]); }
Dbl QuadD2::area() C { return 0.5 * QuadArea2(p[0], p[1], p[2], p[3]); }
Flt Quad ::area() C { return 0.5f * QuadArea2(p[0], p[1], p[2], p[3]); }
Dbl QuadD ::area() C { return 0.5 * QuadArea2(p[0], p[1], p[2], p[3]); }
/******************************************************************************/
static inline Bool SameSide(Int a, Int b) { return a == b || !a || !b; } // both are on same side or one is zero, the same as "a*b>=0"
static Bool SameSide(C Int *x, Int elms) {
    Int last = 0;
    REP(elms)
    if (Int v = *x++)
        if (!last)
            last = v;
        else if (last != v)
            return false;
    return true;
}
Bool Quad2::convex() C // this method supports non-clockwise ordering (in such case it will treat the quad as if it's in clockwise ordering)
{
    Int s[] = {Sign(Cross(p[1] - p[0], p[3] - p[0])),
               Sign(Cross(p[2] - p[1], p[0] - p[1])),
               Sign(Cross(p[3] - p[2], p[1] - p[2])),
               Sign(Cross(p[0] - p[3], p[2] - p[3]))};
    return SameSide(s, Elms(s));
}
Bool Quad::convex() C // this method supports non-clockwise ordering (in such case it will treat the quad as if it's in clockwise ordering)
{
    Vec nu = getNormalU(); // can use 'getNormalU' because here we just need the sign
    Int s[] = {Sign(Dot(nu, Cross(p[1] - p[0], p[3] - p[0]))),
               Sign(Dot(nu, Cross(p[2] - p[1], p[0] - p[1]))),
               Sign(Dot(nu, Cross(p[3] - p[2], p[1] - p[2]))),
               Sign(Dot(nu, Cross(p[0] - p[3], p[2] - p[3])))};
    return SameSide(s, Elms(s));
}
Bool QuadN::convex() C // this method supports non-clockwise ordering (in such case it will treat the quad as if it's in clockwise ordering)
{
    Int s[] = {Sign(Dot(n, Cross(p[1] - p[0], p[3] - p[0]))),
               Sign(Dot(n, Cross(p[2] - p[1], p[0] - p[1]))),
               Sign(Dot(n, Cross(p[3] - p[2], p[1] - p[2]))),
               Sign(Dot(n, Cross(p[0] - p[3], p[2] - p[3])))};
    return SameSide(s, Elms(s));
}
Bool Quad2::valid(Flt eps) C {
    return DistPointLine(p[0], p[1], !(p[3] - p[1])) > eps && DistPointLine(p[1], p[2], !(p[0] - p[2])) > eps && DistPointLine(p[2], p[3], !(p[1] - p[3])) > eps && DistPointLine(p[3], p[0], !(p[2] - p[0])) > eps;
}
Bool Quad::valid(Flt eps) C {
    return DistPointLine(p[0], p[1], !(p[3] - p[1])) > eps && DistPointLine(p[1], p[2], !(p[0] - p[2])) > eps && DistPointLine(p[2], p[3], !(p[1] - p[3])) > eps && DistPointLine(p[3], p[0], !(p[2] - p[0])) > eps;
}
/******************************************************************************/
Bool Quad2 ::clockwise() C { return Cross(p[1] - p[0], p[3] - p[0]) < 0; }
Bool QuadD2::clockwise() C { return Cross(p[1] - p[0], p[3] - p[0]) < 0; }
/******************************************************************************/
Bool Quad::coplanar(C QuadN &quad) C {
    return Abs(DistPointPlane(p[0], quad)) <= EPS && Abs(DistPointPlane(p[1], quad)) <= EPS && Abs(DistPointPlane(p[2], quad)) <= EPS && Abs(DistPointPlane(p[3], quad)) <= EPS;
}
Bool QuadN::coplanar(C Quad &quad) C {
    return Abs(DistPointPlane(quad.p[0], T)) <= EPS && Abs(DistPointPlane(quad.p[1], T)) <= EPS && Abs(DistPointPlane(quad.p[2], T)) <= EPS && Abs(DistPointPlane(quad.p[3], T)) <= EPS;
}
/******************************************************************************/
void Quad2::draw(C Color &color, Bool fill) C {
    VI.color(color);
    VI.setType(VI_2D_FLAT, fill ? VI_STRIP : VI_LINE | VI_STRIP);
    if (Vtx2DFlat *v = (Vtx2DFlat *)VI.addVtx(fill ? 4 : 5)) {
        if (fill) {
            v[0].pos = p[0];
            v[1].pos = p[1];
            v[2].pos = p[3];
            v[3].pos = p[2];
        } else {
            v[0].pos = p[0];
            v[1].pos = p[1];
            v[2].pos = p[2];
            v[3].pos = p[3];
            v[4].pos = p[0];
        }
    }
    VI.end();
}
void Quad::draw(C Color &color, Bool fill) C {
    VI.color(color);
    VI.setType(VI_3D_FLAT, fill ? VI_STRIP : VI_LINE | VI_STRIP);
    if (Vtx3DFlat *v = (Vtx3DFlat *)VI.addVtx(fill ? 4 : 5)) {
        if (fill) {
            v[0].pos = p[0];
            v[1].pos = p[1];
            v[2].pos = p[3];
            v[3].pos = p[2];
        } else {
            v[0].pos = p[0];
            v[1].pos = p[1];
            v[2].pos = p[2];
            v[3].pos = p[3];
            v[4].pos = p[0];
        }
    }
    VI.end();
}
/******************************************************************************/
static inline DIST_TYPE UpdateTypeQ(DIST_TYPE type, DIST_TYPE p0, DIST_TYPE p1, DIST_TYPE edge) {
    switch (type) {
    case DIST_POINT0:
        return p0;
    case DIST_POINT1:
        return p1;
    case DIST_EDGE:
        return edge;
    default:
        return type;
    }
}
Flt Dist(C Vec2 &point, C Quad2 &quad, DIST_TYPE *_type) {
    DIST_TYPE t, type = DIST_NONE;
    Flt d, dist = 0;
    UInt sign = (quad.clockwise() ? 0 : SIGN_BIT); // counter-clockwise tris require changing sign

    Vec2 p = point - quad.p[1];
    if (Xor(DistPointPlane(p, Perp(quad.p[0] - quad.p[1])), sign) > 0) {
        d = DistPointEdge(point, quad.p[0], quad.p[1], &t);
        if (!type || d < dist) {
            dist = d;
            type = UpdateTypeQ(t, DIST_POINT0, DIST_POINT1, DIST_EDGE0);
        }
    }
    if (Xor(DistPointPlane(p, Perp(quad.p[1] - quad.p[2])), sign) > 0) {
        d = DistPointEdge(point, quad.p[1], quad.p[2], &t);
        if (!type || d < dist) {
            dist = d;
            type = UpdateTypeQ(t, DIST_POINT1, DIST_POINT2, DIST_EDGE1);
        }
    }
    p = point - quad.p[3];
    if (Xor(DistPointPlane(p, Perp(quad.p[2] - quad.p[3])), sign) > 0) {
        d = DistPointEdge(point, quad.p[2], quad.p[3], &t);
        if (!type || d < dist) {
            dist = d;
            type = UpdateTypeQ(t, DIST_POINT2, DIST_POINT3, DIST_EDGE2);
        }
    }
    if (Xor(DistPointPlane(p, Perp(quad.p[3] - quad.p[0])), sign) > 0) {
        d = DistPointEdge(point, quad.p[3], quad.p[0], &t);
        if (!type || d < dist) {
            dist = d;
            type = UpdateTypeQ(t, DIST_POINT3, DIST_POINT0, DIST_EDGE3);
        }
    }
    if (!type) {
        type = DIST_PLANE;
        dist = 0;
    }
    if (_type)
        *_type = type;
    return dist;
}
Flt Dist(C Vec &point, C QuadN &quad, DIST_TYPE *_type, Bool test_quads_as_2_tris) {
    DIST_TYPE t, type = DIST_NONE;
    Flt d, dist = 0;
    if (test_quads_as_2_tris) {
        dist = Dist(point, quad.Quad::tri013(), &type); // use base class func to recalc normal
        switch (type) {
        case DIST_POINT2:
            type = DIST_POINT3;
            break;
        case DIST_EDGE1:
            type = DIST_PLANE;
            break;
        case DIST_EDGE2:
            type = DIST_EDGE3;
            break;
        }
        d = Dist(point, quad.Quad::tri123(), &t); // use base class func to recalc normal
        if (d < dist) {
            dist = d;
            switch (type = t) {
            case DIST_POINT0:
                type = DIST_POINT1;
                break;
            case DIST_POINT1:
                type = DIST_POINT2;
                break;
            case DIST_POINT2:
                type = DIST_POINT3;
                break;
            case DIST_EDGE0:
                type = DIST_EDGE1;
                break;
            case DIST_EDGE1:
                type = DIST_EDGE2;
                break;
            case DIST_EDGE2:
                type = DIST_PLANE;
                break;
            }
        }
    } else {
        Vec p = point - quad.p[1];
        if (DistPointPlane(p, Cross(quad.n, quad.p[0] - quad.p[1])) > 0) {
            d = DistPointEdge(point, quad.p[0], quad.p[1], &t);
            if (!type || d < dist) {
                dist = d;
                type = UpdateTypeQ(t, DIST_POINT0, DIST_POINT1, DIST_EDGE0);
            }
        }
        if (DistPointPlane(p, Cross(quad.n, quad.p[1] - quad.p[2])) > 0) {
            d = DistPointEdge(point, quad.p[1], quad.p[2], &t);
            if (!type || d < dist) {
                dist = d;
                type = UpdateTypeQ(t, DIST_POINT1, DIST_POINT2, DIST_EDGE1);
            }
        }
        p = point - quad.p[3];
        if (DistPointPlane(p, Cross(quad.n, quad.p[2] - quad.p[3])) > 0) {
            d = DistPointEdge(point, quad.p[2], quad.p[3], &t);
            if (!type || d < dist) {
                dist = d;
                type = UpdateTypeQ(t, DIST_POINT2, DIST_POINT3, DIST_EDGE2);
            }
        }
        if (DistPointPlane(p, Cross(quad.n, quad.p[3] - quad.p[0])) > 0) {
            d = DistPointEdge(point, quad.p[3], quad.p[0], &t);
            if (!type || d < dist) {
                dist = d;
                type = UpdateTypeQ(t, DIST_POINT3, DIST_POINT0, DIST_EDGE3);
            }
        }
        if (!type) {
            type = DIST_PLANE;
            dist = Abs(DistPointPlane(point, quad));
        }
    }
    if (_type)
        *_type = type;
    return dist;
}
/******************************************************************************/
Bool Cuts(C Vec2 &point, C Quad2 &quad) {
    UInt sign = (quad.clockwise() ? 0 : SIGN_BIT); // counter-clockwise tris require changing sign
    Vec2 p = point - quad.p[1];
    if (Xor(DistPointPlane(p, Perp(quad.p[0] - quad.p[1])), sign) > 0)
        return false;
    if (Xor(DistPointPlane(p, Perp(quad.p[1] - quad.p[2])), sign) > 0)
        return false;
    p = point - quad.p[3];
    if (Xor(DistPointPlane(p, Perp(quad.p[2] - quad.p[3])), sign) > 0)
        return false;
    if (Xor(DistPointPlane(p, Perp(quad.p[3] - quad.p[0])), sign) > 0)
        return false;
    return true;
}
Bool Cuts(C Vec &point, C Quad &quad, Bool test_quads_as_2_tris) {
    if (test_quads_as_2_tris) {
        return Cuts(point, quad.Quad::tri013()) || Cuts(point, quad.Quad::tri123()); // use base class func to recalc normal
    } else {
        Vec quad_nu = quad.getNormalU(); // can use 'getNormalU' because here we just need the sign
        Vec p = point - quad.p[1];
        if (DistPointPlane(p, Cross(quad_nu, quad.p[0] - quad.p[1])) > 0)
            return false;
        if (DistPointPlane(p, Cross(quad_nu, quad.p[1] - quad.p[2])) > 0)
            return false;
        p = point - quad.p[3];
        if (DistPointPlane(p, Cross(quad_nu, quad.p[2] - quad.p[3])) > 0)
            return false;
        if (DistPointPlane(p, Cross(quad_nu, quad.p[3] - quad.p[0])) > 0)
            return false;
    }
    return true;
}
Bool Cuts(C Vec &point, C QuadN &quad, Bool test_quads_as_2_tris) {
    if (test_quads_as_2_tris) {
        return Cuts(point, quad.Quad::tri013()) || Cuts(point, quad.Quad::tri123()); // use base class func to recalc normal
    } else {
        Vec p = point - quad.p[1];
        if (DistPointPlane(p, Cross(quad.n, quad.p[0] - quad.p[1])) > 0)
            return false;
        if (DistPointPlane(p, Cross(quad.n, quad.p[1] - quad.p[2])) > 0)
            return false;
        p = point - quad.p[3];
        if (DistPointPlane(p, Cross(quad.n, quad.p[2] - quad.p[3])) > 0)
            return false;
        if (DistPointPlane(p, Cross(quad.n, quad.p[3] - quad.p[0])) > 0)
            return false;
    }
    return true;
}
Bool CutsEps(C Vec2 &point, C Quad2 &quad) {
    UInt sign = (quad.clockwise() ? 0 : SIGN_BIT); // counter-clockwise tris require changing sign
    Vec2 d10 = !(quad.p[0] - quad.p[1]), p0 = point - quad.p[0];
    if (Xor(DistPointPlane(p0, Perp(d10)), sign) > EPS)
        return false;
    Vec2 d21 = !(quad.p[1] - quad.p[2]), p1 = point - quad.p[1];
    if (Xor(DistPointPlane(p1, Perp(d21)), sign) > EPS)
        return false;
    Vec2 d32 = !(quad.p[2] - quad.p[3]), p2 = point - quad.p[2];
    if (Xor(DistPointPlane(p2, Perp(d32)), sign) > EPS)
        return false;
    Vec2 d03 = !(quad.p[3] - quad.p[0]), p3 = point - quad.p[3];
    if (Xor(DistPointPlane(p3, Perp(d03)), sign) > EPS)
        return false;
    if (DistPointPlane(p0, !(d10 - d03)) > EPS)
        return false;
    if (DistPointPlane(p1, !(d21 - d10)) > EPS)
        return false;
    if (DistPointPlane(p2, !(d32 - d21)) > EPS)
        return false;
    if (DistPointPlane(p3, !(d03 - d32)) > EPS)
        return false;
    return true;
}
Bool CutsEps(C Vec &point, C QuadN &quad, Bool test_quads_as_2_tris) {
    if (test_quads_as_2_tris) {
        return Cuts(point, quad.Quad::tri013()) || Cuts(point, quad.Quad::tri123()); // use base class func to recalc normal
    } else {
        Vec d10 = !(quad.p[0] - quad.p[1]), p0 = point - quad.p[0];
        if (DistPointPlane(p0, Cross(quad.n, d10)) > EPS)
            return false;
        Vec d21 = !(quad.p[1] - quad.p[2]), p1 = point - quad.p[1];
        if (DistPointPlane(p1, Cross(quad.n, d21)) > EPS)
            return false;
        Vec d32 = !(quad.p[2] - quad.p[3]), p2 = point - quad.p[2];
        if (DistPointPlane(p2, Cross(quad.n, d32)) > EPS)
            return false;
        Vec d03 = !(quad.p[3] - quad.p[0]), p3 = point - quad.p[3];
        if (DistPointPlane(p3, Cross(quad.n, d03)) > EPS)
            return false;
        if (DistPointPlane(p0, !(d10 - d03)) > EPS)
            return false;
        if (DistPointPlane(p1, !(d21 - d10)) > EPS)
            return false;
        if (DistPointPlane(p2, !(d32 - d21)) > EPS)
            return false;
        if (DistPointPlane(p3, !(d03 - d32)) > EPS)
            return false;
    }
    return true;
}
/******************************************************************************/
Bool SweepPointQuad(C Vec &point, C Vec &move, C QuadN &quad, Flt *hit_frac, Vec *hit_pos, Bool test_quads_as_2_tris, Bool two_sided) {
    if (test_quads_as_2_tris) {
        return SweepPointTri(point, move, quad.Quad::tri013(), hit_frac, hit_pos, two_sided)     // use base class func to recalc normal
               || SweepPointTri(point, move, quad.Quad::tri123(), hit_frac, hit_pos, two_sided); // use base class func to recalc normal
    } else {
        Vec hitp;
        if (SweepPointPlane(point, move, Plane(quad.p[0], quad.n), hit_frac, null, &hitp, two_sided) && Cuts(hitp, quad)) {
            if (hit_pos)
                *hit_pos = hitp;
            return true;
        }
    }
    return false;
}
Bool SweepPointQuadEps(C Vec &point, C Vec &move, C QuadN &quad, Flt *hit_frac, Vec *hit_pos, Bool test_quads_as_2_tris, Bool two_sided) {
    if (test_quads_as_2_tris) {
        return SweepPointTriEps(point, move, quad.Quad::tri013(), hit_frac, hit_pos, two_sided)     // use base class func to recalc normal
               || SweepPointTriEps(point, move, quad.Quad::tri123(), hit_frac, hit_pos, two_sided); // use base class func to recalc normal
    } else {
        Vec hitp;
        if (SweepPointPlane(point, move, Plane(quad.p[0], quad.n), hit_frac, null, &hitp, two_sided) && CutsEps(hitp, quad)) {
            if (hit_pos)
                *hit_pos = hitp;
            return true;
        }
    }
    return false;
}
Bool CutsLineQuadEps(C Vec &line_pos, C Vec &line_dir, C QuadN &quad, Flt *hit_frac, Vec *hit_pos, Bool test_quads_as_2_tris, Bool two_sided) {
    if (test_quads_as_2_tris) {
        return CutsLineTriEps(line_pos, line_dir, quad.Quad::tri013(), hit_frac, hit_pos, two_sided)     // use base class func to recalc normal
               || CutsLineTriEps(line_pos, line_dir, quad.Quad::tri123(), hit_frac, hit_pos, two_sided); // use base class func to recalc normal
    } else {
        Vec hitp;
        if (CutsLinePlane(line_pos, line_dir, Plane(quad.p[0], quad.n), hit_frac, null, &hitp, two_sided) && CutsEps(hitp, quad)) {
            if (hit_pos)
                *hit_pos = hitp;
            return true;
        }
    }
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
