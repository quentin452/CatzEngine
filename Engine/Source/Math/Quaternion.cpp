/******************************************************************************/
#include "../../stdafx.h"
namespace EE {
/******************************************************************************/
void Quaternion::mul(C Quaternion &q, Quaternion &dest) C {
    dest.set(z * q.y + w * q.x + x * q.w - y * q.z,
             x * q.z + w * q.y + y * q.w - z * q.x,
             y * q.x + w * q.z + z * q.w - x * q.y,
             w * q.w - x * q.x - y * q.y - z * q.z);
}
void QuaternionD::mul(C QuaternionD &q, QuaternionD &dest) C {
    dest.set(z * q.y + w * q.x + x * q.w - y * q.z,
             x * q.z + w * q.y + y * q.w - z * q.x,
             y * q.x + w * q.z + z * q.w - x * q.y,
             w * q.w - x * q.x - y * q.y - z * q.z);
}
void Quaternion ::inverseNormalized(Quaternion &dest) C { dest.set(-x, -y, -z, w); }
void QuaternionD::inverseNormalized(QuaternionD &dest) C { dest.set(-x, -y, -z, w); }
void Quaternion ::inverse(Quaternion &dest) C {
    dest.set(-x, -y, -z, w);
    dest /= length2();
}
void QuaternionD::inverse(QuaternionD &dest) C {
    dest.set(-x, -y, -z, w);
    dest /= length2();
}
/******************************************************************************/
Quaternion &Quaternion ::setRotate(C Vec &axis, Flt angle) {
    Flt sin;
    CosSin(w, sin, angle * 0.5f);
    xyz = axis * sin;
    return T;
}
QuaternionD &QuaternionD::setRotate(C VecD &axis, Dbl angle) {
    Dbl sin;
    CosSin(w, sin, angle * 0.5);
    xyz = axis * sin;
    return T;
}

Quaternion &Quaternion ::setRotateX(Flt angle) {
    CosSin(w, x, angle * 0.5f);
    y = z = 0;
    return T;
}
QuaternionD &QuaternionD::setRotateX(Dbl angle) {
    CosSin(w, x, angle * 0.5);
    y = z = 0;
    return T;
}
Quaternion &Quaternion ::setRotateY(Flt angle) {
    CosSin(w, y, angle * 0.5f);
    x = z = 0;
    return T;
}
QuaternionD &QuaternionD::setRotateY(Dbl angle) {
    CosSin(w, y, angle * 0.5);
    x = z = 0;
    return T;
}
Quaternion &Quaternion ::setRotateZ(Flt angle) {
    CosSin(w, z, angle * 0.5f);
    x = y = 0;
    return T;
}
QuaternionD &QuaternionD::setRotateZ(Dbl angle) {
    CosSin(w, z, angle * 0.5);
    x = y = 0;
    return T;
}

Quaternion &Quaternion::setRotate(Flt x, Flt y, Flt z) {
    Flt cos_x_2, sin_x_2;
    CosSin(cos_x_2, sin_x_2, x * 0.5f);
    Flt cos_y_2, sin_y_2;
    CosSin(cos_y_2, sin_y_2, y * 0.5f);
    Flt cos_z_2, sin_z_2;
    CosSin(cos_z_2, sin_z_2, z * 0.5f);

    T.x = cos_z_2 * cos_y_2 * sin_x_2 + sin_z_2 * sin_y_2 * cos_x_2;
    T.y = cos_z_2 * sin_y_2 * cos_x_2 - sin_z_2 * cos_y_2 * sin_x_2;
    T.z = sin_z_2 * cos_y_2 * cos_x_2 + cos_z_2 * sin_y_2 * sin_x_2;
    T.w = cos_z_2 * cos_y_2 * cos_x_2 - sin_z_2 * sin_y_2 * sin_x_2;

    return T;
}
QuaternionD &QuaternionD::setRotate(Dbl x, Dbl y, Dbl z) {
    Dbl cos_x_2, sin_x_2;
    CosSin(cos_x_2, sin_x_2, x * 0.5);
    Dbl cos_y_2, sin_y_2;
    CosSin(cos_y_2, sin_y_2, y * 0.5);
    Dbl cos_z_2, sin_z_2;
    CosSin(cos_z_2, sin_z_2, z * 0.5);

    T.x = cos_z_2 * cos_y_2 * sin_x_2 + sin_z_2 * sin_y_2 * cos_x_2;
    T.y = cos_z_2 * sin_y_2 * cos_x_2 - sin_z_2 * cos_y_2 * sin_x_2;
    T.z = sin_z_2 * cos_y_2 * cos_x_2 + cos_z_2 * sin_y_2 * sin_x_2;
    T.w = cos_z_2 * cos_y_2 * cos_x_2 - sin_z_2 * sin_y_2 * sin_x_2;

    return T;
}
/******************************************************************************/
Flt Quaternion ::angle() C {
    Flt angle = ACosSin(w, xyz.length()) * 2;
    return (angle > PI) ? angle - PI2 : angle;
} // Quaternion must be normalized, using 'ACosSin' is much more precise than just "Acos(w)", wrap to -PI..PI range !! needed for 'GetDelta' to calculate correct velocities !!
Dbl QuaternionD::angle() C {
    Dbl angle = ACosSin(w, xyz.length()) * 2;
    return (angle > PID) ? angle - PID2 : angle;
} // Quaternion must be normalized, using 'ACosSin' is much more precise than just "Acos(w)", wrap to -PI..PI range !! needed for 'GetDelta' to calculate correct velocities !!

Vec Quaternion ::axis() C {
    Vec axis = xyz;
    axis.normalize();
    return axis;
}
VecD QuaternionD::axis() C {
    VecD axis = xyz;
    axis.normalize();
    return axis;
}

Vec Quaternion ::axisAngle() C {
    Flt length = xyz.length(), angle = ACosSin(w, length) * 2;
    if (angle > PI)
        angle -= PI2;
    return length ? xyz * (angle / length) : VecZero;
}
VecD QuaternionD::axisAngle() C {
    Dbl length = xyz.length(), angle = ACosSin(w, length) * 2;
    if (angle > PID)
        angle -= PID2;
    return length ? xyz * (angle / length) : VecDZero;
}
/******************************************************************************/
Orient::Orient(C Quaternion &q) {
    Flt xx = q.x * q.x,
        yy = q.y * q.y,
        zz = q.z * q.z,
        ww = q.w * q.w;

    // 'mul' (inverse square length) is only required if quaternion is not already normalized
    Flt mul = 1 / (xx + yy + zz + ww);
    // T.cross.x=( xx-yy-zz+ww)*mul;
    T.perp.y = (-xx + yy - zz + ww) * mul;
    T.dir.z = (-xx - yy + zz + ww) * mul;

    mul *= 2;

    Flt a = q.x * q.y,
        b = q.z * q.w;
    T.perp.x = (a - b) * mul;
    // T.cross.y=(a+b)*mul;

    a = q.x * q.z;
    b = q.y * q.w;
    T.dir.x = (a + b) * mul;
    // T.cross.z=(a-b)*mul;

    a = q.y * q.z;
    b = q.x * q.w;
    T.dir.y = (a - b) * mul;
    T.perp.z = (a + b) * mul;
}
Matrix3::Matrix3(C Quaternion &q) {
    Flt xx = q.x * q.x,
        yy = q.y * q.y,
        zz = q.z * q.z,
        ww = q.w * q.w;

    // 'mul' (inverse square length) is only required if quaternion is not already normalized
    Flt mul = 1 / (xx + yy + zz + ww);
    T.x.x = (xx - yy - zz + ww) * mul;
    T.y.y = (-xx + yy - zz + ww) * mul;
    T.z.z = (-xx - yy + zz + ww) * mul;

    mul *= 2;

    Flt a = q.x * q.y,
        b = q.z * q.w;
    T.y.x = (a - b) * mul;
    T.x.y = (a + b) * mul;

    a = q.x * q.z;
    b = q.y * q.w;
    T.z.x = (a + b) * mul;
    T.x.z = (a - b) * mul;

    a = q.y * q.z;
    b = q.x * q.w;
    T.z.y = (a - b) * mul;
    T.y.z = (a + b) * mul;
}
MatrixD3::MatrixD3(C QuaternionD &q) {
    Dbl xx = q.x * q.x,
        yy = q.y * q.y,
        zz = q.z * q.z,
        ww = q.w * q.w;

    // 'mul' (inverse square length) is only required if quaternion is not already normalized
    Dbl mul = 1 / (xx + yy + zz + ww);
    T.x.x = (xx - yy - zz + ww) * mul;
    T.y.y = (-xx + yy - zz + ww) * mul;
    T.z.z = (-xx - yy + zz + ww) * mul;

    mul *= 2;

    Dbl a = q.x * q.y,
        b = q.z * q.w;
    T.y.x = (a - b) * mul;
    T.x.y = (a + b) * mul;

    a = q.x * q.z;
    b = q.y * q.w;
    T.z.x = (a + b) * mul;
    T.x.z = (a - b) * mul;

    a = q.y * q.z;
    b = q.x * q.w;
    T.z.y = (a - b) * mul;
    T.y.z = (a + b) * mul;
}
/******************************************************************************/
Quaternion::Quaternion(C Matrix3 &m) {
    Flt t;
    if (m.z.z < 0) {
        if (m.x.x > m.y.y) {
            t = 1 + m.x.x - m.y.y - m.z.z;
            set(t, m.x.y + m.y.x, m.z.x + m.x.z, m.y.z - m.z.y);
        } else {
            t = 1 - m.x.x + m.y.y - m.z.z;
            set(m.x.y + m.y.x, t, m.y.z + m.z.y, m.z.x - m.x.z);
        }
    } else {
        if (m.x.x < -m.y.y) {
            t = 1 - m.x.x - m.y.y + m.z.z;
            set(m.z.x + m.x.z, m.y.z + m.z.y, t, m.x.y - m.y.x);
        } else {
            t = 1 + m.x.x + m.y.y + m.z.z;
            set(m.y.z - m.z.y, m.z.x - m.x.z, m.x.y - m.y.x, t);
        }
    }
    T *= 0.5f / SqrtFast(t);
}
QuaternionD::QuaternionD(C MatrixD3 &m) {
    Dbl t;
    if (m.z.z < 0) {
        if (m.x.x > m.y.y) {
            t = 1 + m.x.x - m.y.y - m.z.z;
            set(t, m.x.y + m.y.x, m.z.x + m.x.z, m.y.z - m.z.y);
        } else {
            t = 1 - m.x.x + m.y.y - m.z.z;
            set(m.x.y + m.y.x, t, m.y.z + m.z.y, m.z.x - m.x.z);
        }
    } else {
        if (m.x.x < -m.y.y) {
            t = 1 - m.x.x - m.y.y + m.z.z;
            set(m.z.x + m.x.z, m.y.z + m.z.y, t, m.x.y - m.y.x);
        } else {
            t = 1 + m.x.x + m.y.y + m.z.z;
            set(m.y.z - m.z.y, m.z.x - m.x.z, m.x.y - m.y.x, t);
        }
    }
    T *= 0.5 / SqrtFast(t);
}
/******************************************************************************/
static Quaternion Log(C Quaternion &q) {
    Flt length = q.xyz.length();
    if (length <= EPS)
        return Vec4(q.xyz, 0);
    else
        return Vec4(q.xyz * (Acos(q.w) / length), 0);
}
static Quaternion Exp(C Quaternion &q) {
    Flt length = q.xyz.length();
    if (length <= EPS)
        return Vec4(q.xyz, Cos(length));
    else
        return Vec4(q.xyz * (Sin(length) / length), Cos(length));
}
Quaternion GetTangent(C Quaternion &prev, C Quaternion &cur, C Quaternion &next) {
    Quaternion cur_inv;
    cur.inverseNormalized(cur_inv);

    Quaternion logs = (Log(cur_inv * prev) + Log(cur_inv * next)) * -0.25f;
    Quaternion O;
    cur.mul(Exp(logs), O);
    return O;
}
/******************************************************************************/
static Quaternion SlerpNoInv(C Quaternion &a, C Quaternion &b, Flt step) {
    Quaternion O;
    Flt dot = Dot(a, b);
    if (Abs(dot) >= 0.99f)
        O = Lerp(a, b, step);
    else // if angle is small then use linear interpolation !! this also allows to use 'AcosFast' below !!
    {
        Flt angle = AcosFast(dot),
            sin = Sin(angle);
        O = a * (Sin(angle * (1 - step)) / sin) + b * (Sin(angle * step) / sin);
    }
    O.normalize();
    return O;
}
/******************************************************************************/
Quaternion Slerp(C Quaternion &a, C Quaternion &b, Flt step) {
    Quaternion O, temp = b;
    Flt dot = Dot(a, b);
    if (dot < 0) // other side
    {
        CHS(dot);
        temp.chs();
    }
    if (dot >= 0.99f)
        O = Lerp(a, temp, step);
    else // if angle is small then use linear interpolation !! this also allows to use 'AcosFast' below !!
    {
        Flt angle = AcosFast(dot),
            sin = Sin(angle);
        O = a * (Sin(angle * (1 - step)) / sin) + temp * (Sin(angle * step) / sin);
    }
    O.normalize();
    return O;
}
/******************************************************************************/
Quaternion Squad(C Quaternion &from, C Quaternion &to, C Quaternion &tan0, C Quaternion &tan1, Flt step) {
    return SlerpNoInv(SlerpNoInv(from, to, step),
                      SlerpNoInv(tan0, tan1, step), 2 * step * (1 - step));
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
