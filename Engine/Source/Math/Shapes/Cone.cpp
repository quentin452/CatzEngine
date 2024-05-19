/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************/
Flt Cone::area() C {
    Flt min, max;
    MinMax(r_low, r_high, min, max);
    if (min == 0)
        return PI * max * (max + Dist(max, h));
    if (min == max)
        return PI2 * max * (max + h); // tube
    Flt h0 = h * min / (max - min);
    return PI * (max * (max + Dist(max, h0 + h)) + min * (min - Dist(min, h0)));
}
Flt ConeM::area() C {
    Flt min, max;
    MinMax(r_low, r_high, min, max);
    if (min == 0)
        return PI * max * (max + Dist(max, h));
    if (min == max)
        return PI2 * max * (max + h); // tube
    Flt h0 = h * min / (max - min);
    return PI * (max * (max + Dist(max, h0 + h)) + min * (min - Dist(min, h0)));
}
Flt Cone::volume() C {
    Flt min, max;
    MinMax(r_low, r_high, min, max);
    if (min == 0)
        return PI_3 * max * max * h;
    if (min == max)
        return PI * max * max * h; // tube
    Flt h0 = h * min / (max - min);
    return PI_3 * (max * max * (h0 + h) - min * min * h0);
}
Flt ConeM::volume() C {
    Flt min, max;
    MinMax(r_low, r_high, min, max);
    if (min == 0)
        return PI_3 * max * max * h;
    if (min == max)
        return PI * max * max * h; // tube
    Flt h0 = h * min / (max - min);
    return PI_3 * (max * max * (h0 + h) - min * min * h0);
}
Cone &Cone::operator*=(Flt f) {
    r_low *= f;
    r_high *= f;
    h *= f;
    pos *= f;
    return T;
}
Cone &Cone::operator/=(Flt f) {
    r_low /= f;
    r_high /= f;
    h /= f;
    pos /= f;
    return T;
}
Cone &Cone::operator*=(C Matrix3 &m) {
    Flt s = m.maxScale();
    pos *= m;
    up *= m;
    h *= up.normalize();
    r_low *= s;
    r_high *= s;
    return T;
}
Cone &Cone::operator*=(C Matrix &m) {
    Flt s = m.maxScale();
    pos *= m;
    up *= m.orn();
    h *= up.normalize();
    r_low *= s;
    r_high *= s;
    return T;
}
/******************************************************************************/
void Cone::draw(C Color &color, Bool fill, Int resolution) C {
    if (resolution < 0)
        resolution = 24;
    else if (resolution < 3)
        resolution = 3;
    Matrix3 matrix;
    matrix.setUp(up);
    matrix.y *= h;
    Vec prev0 = pos + r_low * matrix.x,
        prev1 = pos + matrix.y + r_high * matrix.x;

    VI.color(color);
    REP(resolution) {
        Flt c, s;
        CosSin(c, s, (PI2 * i) / resolution);
        Vec next0 = pos + (c * r_low) * matrix.x + (s * r_low) * matrix.z,
            next1 = pos + matrix.y + (c * r_high) * matrix.x + (s * r_high) * matrix.z;
        if (fill)
            VI.quad(next1, prev1, prev0, next0);
        else {
            VI.line(next1, prev1);
            VI.line(next0, next1);
            VI.line(next0, prev0);
        }
        prev0 = next0;
        prev1 = next1;
    }
    VI.end();
}
/******************************************************************************/
Bool Cuts(C Vec &point, C Cone &cone) {
    Flt z = DistPointPlane(point, cone.pos, cone.up);
    if (z >= 0 && z <= cone.h) {
        Vec on_line = cone.pos + cone.up * z;
        Flt dist2 = Dist2(point, on_line),
            r2 = Sqr((cone.h > 0) ? Lerp(cone.r_low, cone.r_high, z / cone.h) : Max(cone.r_low, cone.r_high));
        return dist2 <= r2;
    }
    return false;
}
Bool Cuts(C VecD &point, C ConeM &cone) {
    Dbl z = DistPointPlane(point, cone.pos, cone.up);
    if (z >= 0 && z <= cone.h) {
        VecD on_line = cone.pos + cone.up * z;
        Flt dist2 = Dist2(point, on_line),
            r2 = Sqr((cone.h > 0) ? Lerp(cone.r_low, cone.r_high, Flt(z) / cone.h) : Max(cone.r_low, cone.r_high));
        return dist2 <= r2;
    }
    return false;
}
/******************************************************************************/
Flt Dist(C Vec &point, C Cone &cone) {
    Flt z = DistPointPlane(point, cone.pos, cone.up);
    Vec on_line = cone.pos + cone.up * z,
        delta = point - on_line;
    Flt delta_len = delta.length();
    if (z <= 0) {
        if (delta_len <= cone.r_low)
            return -z; // distance between point and low circle
    } else if (z >= cone.h) {
        if (delta_len <= cone.r_high)
            return z - cone.h; // distance between point and high circle
    } else                     // z>0 && z<cone.h
    {
        Flt r = ((cone.h > 0) ? Lerp(cone.r_low, cone.r_high, z / cone.h) : Max(cone.r_low, cone.r_high));
        if (delta_len <= r)
            return 0; // cuts
    }
    // at this stage 'delta_len' should be >0 (because for all 3 possibilities of 'z' being under, above or inside cone, we return if 'delta_len'<=radius)
    Vec low = cone.pos + delta * (cone.r_low / delta_len),
        high = cone.pos + cone.h * cone.up + delta * (cone.r_high / delta_len);
    return DistPointEdge(point, low, high);
}
Dbl Dist(C VecD &point, C ConeM &cone) {
    Dbl z = DistPointPlane(point, cone.pos, cone.up);
    VecD on_line = cone.pos + cone.up * z,
         delta = point - on_line;
    Dbl delta_len = delta.length();
    if (z <= 0) {
        if (delta_len <= cone.r_low)
            return -z; // distance between point and low circle
    } else if (z >= cone.h) {
        if (delta_len <= cone.r_high)
            return z - cone.h; // distance between point and high circle
    } else                     // z>0 && z<cone.h
    {
        Flt r = ((cone.h > 0) ? Lerp(cone.r_low, cone.r_high, Flt(z) / cone.h) : Max(cone.r_low, cone.r_high));
        if (delta_len <= r)
            return 0; // cuts
    }
    // at this stage 'delta_len' should be >0 (because for all 3 possibilities of 'z' being under, above or inside cone, we return if 'delta_len'<=radius)
    VecD low = cone.pos + delta * (cone.r_low / delta_len),
         high = cone.pos + cone.h * cone.up + delta * (cone.r_high / delta_len);
    return DistPointEdge(point, low, high);
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
