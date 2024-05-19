/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'Quaternion' for optional rotation representation.

/******************************************************************************/
struct Quaternion : Vec4 {
    Quaternion &operator*=(Flt f) {
        super::operator*=(f);
        return T;
    }
    Quaternion &operator*=(C Quaternion &q) { return mul(q); }

    friend Quaternion operator*(C Quaternion &a, C Quaternion &b) {
        Quaternion temp;
        a.mul(b, temp);
        return temp;
    }                                                                                    // get a*b
    friend Quaternion operator*(C Quaternion &q, Flt f) { return SCAST(C Vec4, q) * f; } // get q*f
    friend Quaternion operator*(Flt f, C Quaternion &q) { return f * SCAST(C Vec4, q); } // get f*q

    // transform
    void mul(C Quaternion &q, Quaternion &dest) C; // multiply by 'q' and store result in 'dest'
    Quaternion &mul(C Quaternion &q) {
        mul(q, T);
        return T;
    }                                 // multiply by 'q'
    void inverse(Quaternion &dest) C; // inverse to 'dest'
    Quaternion &inverse() {
        inverse(T);
        return T;
    }                                           // inverse
    void inverseNormalized(Quaternion &dest) C; // inverse to 'dest', this method if faster than 'inverse' however Quaternion must be normalized
    Quaternion &inverseNormalized() {
        inverseNormalized(T);
        return T;
    } // inverse, this method if faster than 'inverse' however Quaternion must be normalized
    Quaternion &identity() {
        set(0, 0, 0, 1);
        return T;
    } // set identity

    Quaternion &setRotateX(Flt angle);             // set x     rotated identity
    Quaternion &setRotateY(Flt angle);             // set y     rotated identity
    Quaternion &setRotateZ(Flt angle);             // set z     rotated identity
    Quaternion &setRotate(Flt x, Flt y, Flt z);    // set x-y-z rotated identity
    Quaternion &setRotate(C Vec &axis, Flt angle); // set axis  rotated identity

    // get
    Flt angle() C;     // get rotation angle
    Vec axis() C;      // get rotation axis
    Vec axisAngle() C; // get rotation axis scaled by angle

    Quaternion() {}
    Quaternion(C Vec4 &v) : Vec4(v) {}
    CONVERSION Quaternion(C Matrix3 &m);
};
/******************************************************************************/
struct QuaternionD : VecD4 {
    QuaternionD &operator*=(Dbl f) {
        super::operator*=(f);
        return T;
    }
    QuaternionD &operator*=(C QuaternionD &q) { return mul(q); }

    friend QuaternionD operator*(C QuaternionD &a, C QuaternionD &b) {
        QuaternionD temp;
        a.mul(b, temp);
        return temp;
    }                                                                                       // get a*b
    friend QuaternionD operator*(C QuaternionD &q, Dbl f) { return SCAST(C VecD4, q) * f; } // get q*f
    friend QuaternionD operator*(Dbl f, C QuaternionD &q) { return f * SCAST(C VecD4, q); } // get f*q

    // transform
    void mul(C QuaternionD &q, QuaternionD &dest) C; // multiply by 'q' and store result in 'dest'
    QuaternionD &mul(C QuaternionD &q) {
        mul(q, T);
        return T;
    }                                  // multiply by 'q'
    void inverse(QuaternionD &dest) C; // inverse to 'dest'
    QuaternionD &inverse() {
        inverse(T);
        return T;
    }                                            // inverse
    void inverseNormalized(QuaternionD &dest) C; // inverse to 'dest', this method if faster than 'inverse' however Quaternion must be normalized
    QuaternionD &inverseNormalized() {
        inverseNormalized(T);
        return T;
    } // inverse, this method if faster than 'inverse' however Quaternion must be normalized
    QuaternionD &identity() {
        set(0, 0, 0, 1);
        return T;
    } // set identity

    QuaternionD &setRotateX(Dbl angle);              // set x     rotated identity
    QuaternionD &setRotateY(Dbl angle);              // set y     rotated identity
    QuaternionD &setRotateZ(Dbl angle);              // set z     rotated identity
    QuaternionD &setRotate(Dbl x, Dbl y, Dbl z);     // set x-y-z rotated identity
    QuaternionD &setRotate(C VecD &axis, Dbl angle); // set axis  rotated identity

    // get
    Dbl angle() C;      // get rotation angle
    VecD axis() C;      // get rotation axis
    VecD axisAngle() C; // get rotation axis scaled by angle

    QuaternionD() {}
    QuaternionD(C VecD4 &v) : VecD4(v) {}
    CONVERSION QuaternionD(C MatrixD3 &m);
};
/******************************************************************************/
Quaternion GetTangent(C Quaternion &prev, C Quaternion &cur, C Quaternion &next); // get tangent

Quaternion Slerp(C Quaternion &a, C Quaternion &b, Flt step);                                             // spherical linear     interpolation, 'step'=0..1
Quaternion Squad(C Quaternion &from, C Quaternion &to, C Quaternion &tan0, C Quaternion &tan1, Flt step); // spherical quadrangle interpolation, 'step'=0..1
/******************************************************************************/
