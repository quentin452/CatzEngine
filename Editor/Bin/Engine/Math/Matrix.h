/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'Matrix2' to represent 2D objects orientation and scale.

   Use 'Matrix2P' to represent 2D objects orientation, scale and position.

   Use 'Matrix3' to represent objects orientation and scale.

   Use 'Matrix' to represent objects orientation, scale and position.

   Use 'MatrixM' to represent objects orientation, scale and position (with double precision for position).

   Use 'GetVel' to calculate objects velocities according to its previous and current matrix.

   Use 'SetMatrix' to set mesh rendering matrix (use before manual drawing only).

/******************************************************************************/
struct Matrix2 // Matrix 2x2 (orientation + scale)
{
    Vec2 x, // right vector
        y;  // up    vector

    Matrix2 &operator*=(Flt f);
    Matrix2 &operator/=(Flt f);
    Matrix2 &operator*=(C Vec2 &v);
    Matrix2 &operator/=(C Vec2 &v);
    Matrix2 &operator*=(C Matrix2 &m) { return mul(m); }
    Matrix2 &operator/=(C Matrix2 &m) { return div(m); }
    Bool operator==(C Matrix2 &m) C;
    Bool operator!=(C Matrix2 &m) C;

    friend Matrix2 operator*(C Matrix2 &a, C Matrix2 &b) {
        Matrix2 temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend Matrix2 operator/(C Matrix2 &a, C Matrix2 &b) {
        Matrix2 temp;
        a.div(b, temp);
        return temp;
    } // get a/b
    friend Matrix2 operator~(C Matrix2 &m) {
        Matrix2 temp;
        m.inverse(temp);
        return temp;
    } // get inversed 'm'

    void mul(C Matrix2 &matrix, Matrix2 &dest) C;   // multiply self by 'matrix' and store result in 'dest'
    void mul(C Matrix2P &matrix, Matrix2P &dest) C; // multiply self by 'matrix' and store result in 'dest'
    Matrix2 &mul(C Matrix2 &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'

    void div(C Matrix2 &matrix, Matrix2 &dest) C;   // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    void div(C Matrix2P &matrix, Matrix2P &dest) C; // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    Matrix2 &div(C Matrix2 &matrix) {
        div(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal

    void divNormalized(C Matrix2 &matrix, Matrix2 &dest) C; // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
    Matrix2 &divNormalized(C Matrix2 &matrix) {
        divNormalized(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized

    void inverse(Matrix2 &dest, Bool normalized = false) C; // inverse self to 'dest', if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal
    Matrix2 &inverse(Bool normalized = false) {
        inverse(T, normalized);
        return T;
    } // inverse self          , if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal

    Matrix2 &inverseScale(); // inverse scale

    Matrix2 &normalize();              // normalize scale           , this sets the length of 'x' 'y' vectors to 1
    Matrix2 &normalize(Flt scale);     // normalize scale to 'scale', this sets the length of 'x' 'y' vectors to 'scale'
    Matrix2 &normalize(C Vec2 &scale); // normalize scale to 'scale', this sets the length of 'x' 'y' vectors to 'scale.x' 'scale.y'

    Matrix2 &scale(Flt scale) {
        T *= scale;
        return T;
    } // scale
    Matrix2 &scale(C Vec2 &scale) {
        T *= scale;
        return T;
    }                               // scale
    Matrix2 &scaleL(C Vec2 &scale); // scale in local space

    Matrix2 &rotate(Flt angle); // rotate

    // set (set methods reset the full matrix)
    Matrix2 &identity(); // set as identity
    Matrix2 &zero();     // set all vectors to zero

    Matrix2 &setScale(Flt scale);     // set as scaled identity
    Matrix2 &setScale(C Vec2 &scale); // set as scaled identity

    Matrix2 &setRotate(Flt angle); // set as rotated identity

    // get
    Vec2 scale() C;   // get each    axis scale
    Vec2 scale2() C;  // get each    axis scale squared
    Flt avgScale() C; // get average axis scale
    Flt maxScale() C; // get maximum axis scale

    Matrix2() {}
    Matrix2(Flt scale) { setScale(scale); }
    Matrix2(C Vec2 &scale) { setScale(scale); }
    Matrix2(C Vec2 &x, C Vec2 &y) {
        T.x = x;
        T.y = y;
    }
};
/******************************************************************************/
struct Matrix3 // Matrix 3x3 (orientation + scale)
{
    Vec x, // right   vector
        y, // up      vector
        z; // forward vector

    // transform
    Matrix3 &operator*=(Flt f);
    Matrix3 &operator/=(Flt f);
    Matrix3 &operator*=(C Vec &v);
    Matrix3 &operator/=(C Vec &v);
    Matrix3 &operator+=(C Matrix3 &m);
    Matrix3 &operator-=(C Matrix3 &m);
    Matrix3 &operator*=(C Matrix3 &m) { return mul(m); }
    Matrix3 &operator/=(C Matrix3 &m) { return div(m); }
    Matrix3 &operator*=(C RevMatrix3 &m) { return mul(m); }
    Bool operator==(C Matrix3 &m) C;
    Bool operator!=(C Matrix3 &m) C;

    friend Matrix3 operator*(C Matrix3 &a, C Matrix3 &b) {
        Matrix3 temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend Matrix3 operator*(C Matrix3 &a, C RevMatrix3 &b) {
        Matrix3 temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend Matrix3 operator/(C Matrix3 &a, C Matrix3 &b) {
        Matrix3 temp;
        a.div(b, temp);
        return temp;
    } // get a/b
    friend Matrix3 operator~(C Matrix3 &m) {
        Matrix3 temp;
        m.inverse(temp);
        return temp;
    } // get inversed 'm'

    void mul(C Matrix3 &matrix, Matrix3 &dest) C;    // multiply self by 'matrix' and store result in 'dest'
    void mul(C RevMatrix3 &matrix, Matrix3 &dest) C; // multiply self by 'matrix' and store result in 'dest'
    void mul(C Matrix &matrix, Matrix &dest) C;      // multiply self by 'matrix' and store result in 'dest'
    void mul(C MatrixM &matrix, MatrixM &dest) C;    // multiply self by 'matrix' and store result in 'dest'
    Matrix3 &mul(C Matrix3 &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'
    Matrix3 &mul(C RevMatrix3 &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'

    void div(C Matrix3 &matrix, Matrix3 &dest) C; // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    void div(C Matrix &matrix, Matrix &dest) C;   // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    void div(C MatrixM &matrix, MatrixM &dest) C; // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    Matrix3 &div(C Matrix3 &matrix) {
        div(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal

    void divNormalized(C Matrix3 &matrix, Matrix3 &dest) C; // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
    Matrix3 &divNormalized(C Matrix3 &matrix) {
        divNormalized(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized

    void inverse(Matrix3 &dest, Bool normalized = false) C; // inverse self to 'dest', if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal
    Matrix3 &inverse(Bool normalized = false) {
        inverse(T, normalized);
        return T;
    } // inverse self          , if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal

    void inverseNonOrthogonal(Matrix3 &dest) C; // inverse self to 'dest', this method is slower than 'inverse' however it properly handles non-orthogonal matrixes
    Matrix3 &inverseNonOrthogonal() {
        inverseNonOrthogonal(T);
        return T;
    } // inverse self          , this method is slower than 'inverse' however it properly handles non-orthogonal matrixes

    Matrix3 &inverseScale(); // inverse scale

    Matrix3 &normalize();             // normalize scale           , this sets the length of 'x' 'y' 'z' vectors to 1
    Matrix3 &normalize(Flt scale);    // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale'
    Matrix3 &normalize(C Vec &scale); // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale.x' 'scale.y' 'scale.z'

    Matrix3 &scale(Flt scale) {
        T *= scale;
        return T;
    } // scale
    Matrix3 &scale(C Vec &scale) {
        T *= scale;
        return T;
    }                                           // scale
    Matrix3 &scaleL(C Vec &scale);              // scale in local space
    Matrix3 &scale(C Vec &dir, Flt scale);      // scale along       'dir' direction by 'scale' value, 'dir' must be normalized
    Matrix3 &scalePlane(C Vec &nrm, Flt scale); // scale along plane of 'nrm' normal by 'scale' value, 'nrm' must be normalized

    Matrix3 &rotateX(Flt angle);             // rotate by x axis
    Matrix3 &rotateY(Flt angle);             // rotate by y axis
    Matrix3 &rotateZ(Flt angle);             // rotate by z axis
    Matrix3 &rotateXY(Flt x, Flt y);         // rotate by x axis and then by y axis, works the same as "rotateX(x).rotateY(y)" but faster
    Matrix3 &rotate(C Vec &axis, Flt angle); // rotate by vector, 'axis' must be normalized
    Matrix3 &rotateXL(Flt angle);            // rotate matrix by its x vector (x-axis rotation in local space)
    Matrix3 &rotateYL(Flt angle);            // rotate matrix by its y vector (y-axis rotation in local space)
    Matrix3 &rotateZL(Flt angle);            // rotate matrix by its z vector (z-axis rotation in local space)

    Matrix3 &rotateXLOrthoNormalized(Flt angle); // rotate matrix by its x vector (x-axis rotation in local space), this method is faster than 'rotateXL' however matrix must be orthogonal and normalized
    Matrix3 &rotateYLOrthoNormalized(Flt angle); // rotate matrix by its y vector (y-axis rotation in local space), this method is faster than 'rotateYL' however matrix must be orthogonal and normalized
    Matrix3 &rotateZLOrthoNormalized(Flt angle); // rotate matrix by its z vector (z-axis rotation in local space), this method is faster than 'rotateZL' however matrix must be orthogonal and normalized

    Matrix3 &rotateXLOrthoNormalized(Flt cos, Flt sin); // rotate matrix by its x vector (x-axis rotation in local space), this method works like 'rotateXLOrthoNormalized(Flt angle)' however it accepts 'Cos' and 'Sin' of 'angle'
    Matrix3 &rotateYLOrthoNormalized(Flt cos, Flt sin); // rotate matrix by its y vector (y-axis rotation in local space), this method works like 'rotateYLOrthoNormalized(Flt angle)' however it accepts 'Cos' and 'Sin' of 'angle'

    Matrix3 &rotateToYKeepX(C Vec &y); // rotate matrix to Y direction, try to preserve existing X direction, 'y' must be normalized
    Matrix3 &rotateToYKeepZ(C Vec &y); // rotate matrix to Y direction, try to preserve existing Z direction, 'y' must be normalized
    Matrix3 &rotateToZKeepX(C Vec &z); // rotate matrix to Z direction, try to preserve existing X direction, 'z' must be normalized
    Matrix3 &rotateToZKeepY(C Vec &z); // rotate matrix to Z direction, try to preserve existing Y direction, 'z' must be normalized

    Matrix3 &mirrorX();             // mirror matrix in X axis
    Matrix3 &mirrorY();             // mirror matrix in Y axis
    Matrix3 &mirrorZ();             // mirror matrix in Z axis
    Matrix3 &mirror(C Vec &normal); // mirror matrix by   plane normal

    Matrix3 &swapYZ(); // swap Y and Z components of every vector

    // set (set methods reset the full matrix)
    Matrix3 &identity();          // set as identity
    Matrix3 &identity(Flt blend); // set as identity, this method is similar to 'identity()' however it does not perform full reset of the matrix. Instead, smooth reset is applied depending on 'blend' value (0=no reset, 1=full reset)
    Matrix3 &zero();              // set all vectors to zero
#if EE_PRIVATE
    Matrix3 &keep01(Flt blend); // same as "identity(1-blend)", assumes 0<=blend<=1
#endif

    Matrix3 &setScale(Flt scale);    // set as scaled identity
    Matrix3 &setScale(C Vec &scale); // set as scaled identity

    Matrix3 &setRotateX(Flt angle);             // set as   x-rotated identity
    Matrix3 &setRotateY(Flt angle);             // set as   y-rotated identity
    Matrix3 &setRotateZ(Flt angle);             // set as   z-rotated identity
    Matrix3 &setRotateXY(Flt x, Flt y);         // set as x-y-rotated identity, works the same as 'setRotateX(x).rotateY(y)' but faster
    Matrix3 &setRotate(C Vec &axis, Flt angle); // set as     rotated by vector identity, 'axis' must be normalized
#if EE_PRIVATE
    Matrix3 &setRotateCosSin(C Vec &axis, Flt cos, Flt sin); // set as rotated by vector identity, 'axis' must be normalized
#endif

    Matrix3 &setOrient(DIR_ENUM dir);                                          // set as orientation from DIR_ENUM
    Matrix3 &setRight(C Vec &right);                                           // set as x='right'         and calculate correct y,z, 'right'            must be normalized
    Matrix3 &setRight(C Vec &right, C Vec &up);                                // set as x='right', y='up' and calculate correct z  , 'right' 'up'       must be normalized
    Matrix3 &setUp(C Vec &up);                                                 // set as y='up'            and calculate correct x,z, 'up'               must be normalized
    Matrix3 &setDir(C Vec &dir);                                               // set as z='dir'           and calculate correct x,y, 'dir'              must be normalized
    Matrix3 &setDir(C Vec &dir, C Vec &up);                                    // set as z='dir'  , y='up' and calculate correct x  , 'dir' 'up'         must be normalized
    Matrix3 &setDir(C Vec &dir, C Vec &up, C Vec &right);                      // set as z='dir'  , y='up', x='right'               , 'dir' 'up' 'right' must be normalized
    Matrix3 &setRotation(C Vec &dir_from, C Vec &dir_to);                      // set as matrix which rotates 'dir_from' to 'dir_to',                                              'dir_from' 'dir_to' must be normalized
    Matrix3 &setRotation(C Vec &dir_from, C Vec &dir_to, Flt blend);           // set as matrix which rotates 'dir_from' to 'dir_to', using blend value                          , 'dir_from' 'dir_to' must be normalized
    Matrix3 &setRotation(C Vec &dir_from, C Vec &dir_to, Flt blend, Flt roll); // set as matrix which rotates 'dir_from' to 'dir_to', using blend value and additional roll angle, 'dir_from' 'dir_to' must be normalized
    Matrix3 &setRotationUp(C Vec &dir_to);                                     // set as matrix which rotates Vec(0,1,0) to 'dir_to',                                                         'dir_to' must be normalized

    Matrix3 &setTerrainOrient(DIR_ENUM dir); // set as orientation from DIR_ENUM to be used for drawing spherical terrain heightmaps

    // get
    Flt determinant() C;
    Bool mirrored() C { return determinant() < 0; } // if matrix is mirrored ('x' axis is on the other/left side)

    Vec scale() C;    // get each    axis scale
    Vec scale2() C;   // get each    axis scale squared
    Flt avgScale() C; // get average axis scale
    Flt maxScale() C; // get maximum axis scale

    Vec angles() C;                                      // get rotation angles, this allows to reconstruct the matrix using "setRotateZ(angle.z).rotateX(angle.x).rotateY(angle.y)" or faster by using "setRotateZ(angle.z).rotateXY(angle.x, angle.y)"
    Vec axis(Bool normalized = false) C;                 // get rotation axis                , if you know that the matrix is normalized then set 'normalized=true' for more performance
    Flt angle(Bool normalized = false) C;                // get rotation angle               , if you know that the matrix is normalized then set 'normalized=true' for more performance
    Flt axisAngle(Vec &axis, Bool normalized = false) C; // get rotation axis and       angle, if you know that the matrix is normalized then set 'normalized=true' for more performance
    Vec axisAngle(Bool normalized = false) C;            // get rotation axis scaled by angle, if you know that the matrix is normalized then set 'normalized=true' for more performance
    Flt angleY(Bool normalized = false) C;               // get rotation angle along Y axis  , if you know that the matrix is normalized then set 'normalized=true' for more performance, this is the same as "axisAngle(normalized).y" but faster

    Str asText(Int precision = INT_MAX) C { return S + "X: " + x.asText(precision) + ", Y: " + y.asText(precision) + ", Z:" + z.asText(precision); } // get text description

#if EE_PRIVATE
    // operations
    Vec2 convert(C Vec &src, Bool normalized = false) C;   // return converted 3D 'src' to 2D vector according to matrix x,y axes
    Vec convert(C Vec2 &src) C;                            // return converted 2D 'src' to 3D vector according to matrix x,y axes
    Edge2 convert(C Edge &src, Bool normalized = false) C; // return converted 3D 'src' to 2D edge   according to matrix x,y axes
    Edge convert(C Edge2 &src) C;                          // return converted 2D 'src' to 3D edge   according to matrix x,y axes
#endif

    // draw
    void draw(C Vec &pos, C Color &x_color = RED, C Color &y_color = GREEN, C Color &z_color = BLUE, Bool arrow = true) C; // draw axes, this can be optionally called outside of Render function, this relies on active object matrix which can be set using 'SetMatrix' function

    Matrix3() {}
    Matrix3(Flt scale) { setScale(scale); }
    Matrix3(C Vec &scale) { setScale(scale); }
    Matrix3(C Vec &x, C Vec &y, C Vec &z) {
        T.x = x;
        T.y = y;
        T.z = z;
    }
    CONVERSION Matrix3(C MatrixD3 &m);
    CONVERSION Matrix3(C Matrix &m);
    CONVERSION Matrix3(C Matrix4 &m);
    CONVERSION Matrix3(C Orient &o);
    CONVERSION Matrix3(C OrientD &o);
    CONVERSION Matrix3(C Quaternion &q);
};
/******************************************************************************/
struct MatrixD3 // Matrix 3x3 (orientation + scale, double precision)
{
    VecD x, // right   vector
        y,  // up      vector
        z;  // forward vector

    // transform
    MatrixD3 &operator*=(Dbl f);
    MatrixD3 &operator/=(Dbl f);
    MatrixD3 &operator*=(C VecD &v);
    MatrixD3 &operator/=(C VecD &v);
    MatrixD3 &operator+=(C MatrixD3 &m);
    MatrixD3 &operator-=(C MatrixD3 &m);
    MatrixD3 &operator*=(C Matrix3 &m) { return mul(m); }
    MatrixD3 &operator*=(C MatrixD3 &m) { return mul(m); }
    MatrixD3 &operator/=(C Matrix3 &m) { return div(m); }
    MatrixD3 &operator/=(C MatrixD3 &m) { return div(m); }
    Bool operator==(C MatrixD3 &m) C;
    Bool operator!=(C MatrixD3 &m) C;

    friend MatrixD3 operator*(C MatrixD3 &a, C MatrixD3 &b) {
        MatrixD3 temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend MatrixD3 operator/(C MatrixD3 &a, C MatrixD3 &b) {
        MatrixD3 temp;
        a.div(b, temp);
        return temp;
    } // get a/b
    friend MatrixD3 operator~(C MatrixD3 &m) {
        MatrixD3 temp;
        m.inverse(temp);
        return temp;
    } // get inversed 'm'

    void mul(C Matrix3 &matrix, MatrixD3 &dest) C;  // multiply self by 'matrix' and store result in 'dest'
    void mul(C MatrixD3 &matrix, MatrixD3 &dest) C; // multiply self by 'matrix' and store result in 'dest'
    void mul(C MatrixD &matrix, MatrixD &dest) C;   // multiply self by 'matrix' and store result in 'dest'
    MatrixD3 &mul(C Matrix3 &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'
    MatrixD3 &mul(C MatrixD3 &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'

    void div(C Matrix3 &matrix, MatrixD3 &dest) C;  // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    void div(C MatrixD3 &matrix, MatrixD3 &dest) C; // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    void div(C MatrixD &matrix, MatrixD &dest) C;   // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    MatrixD3 &div(C Matrix3 &matrix) {
        div(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal
    MatrixD3 &div(C MatrixD3 &matrix) {
        div(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal

    void divNormalized(C Matrix3 &matrix, MatrixD3 &dest) C;  // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
    void divNormalized(C MatrixD3 &matrix, MatrixD3 &dest) C; // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
    MatrixD3 &divNormalized(C Matrix3 &matrix) {
        divNormalized(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized
    MatrixD3 &divNormalized(C MatrixD3 &matrix) {
        divNormalized(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized

    void inverse(MatrixD3 &dest, Bool normalized = false) C; // inverse self to 'dest', if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal
    MatrixD3 &inverse(Bool normalized = false) {
        inverse(T, normalized);
        return T;
    } // inverse self          , if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal

    void inverseNonOrthogonal(MatrixD3 &dest) C; // inverse self to 'dest', this method is slower than 'inverse' however it properly handles non-orthogonal matrixes
    MatrixD3 &inverseNonOrthogonal() {
        inverseNonOrthogonal(T);
        return T;
    } // inverse self          , this method is slower than 'inverse' however it properly handles non-orthogonal matrixes

    MatrixD3 &inverseScale(); // inverse scale

    MatrixD3 &normalize();              // normalize scale           , this sets the length of 'x' 'y' 'z' vectors to 1
    MatrixD3 &normalize(Dbl scale);     // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale'
    MatrixD3 &normalize(C VecD &scale); // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale.x' 'scale.y' 'scale.z'

    MatrixD3 &scale(Dbl scale) {
        T *= scale;
        return T;
    } // scale
    MatrixD3 &scale(C VecD &scale) {
        T *= scale;
        return T;
    }                                             // scale
    MatrixD3 &scale(C VecD &dir, Dbl scale);      // scale along       'dir' direction by 'scale' value, 'dir' must be normalized
    MatrixD3 &scalePlane(C VecD &nrm, Dbl scale); // scale along plane of 'nrm' normal by 'scale' value, 'nrm' must be normalized

    MatrixD3 &rotateX(Dbl angle);              // rotate by x axis
    MatrixD3 &rotateY(Dbl angle);              // rotate by y axis
    MatrixD3 &rotateZ(Dbl angle);              // rotate by z axis
    MatrixD3 &rotateXY(Dbl x, Dbl y);          // rotate by x axis and then by y axis, works the same as "rotateX(x).rotateY(y)" but faster
    MatrixD3 &rotate(C VecD &axis, Dbl angle); // rotate by vector, 'axis' must be normalized
    MatrixD3 &rotateXL(Dbl angle);             // rotate matrix by its x vector (x-axis rotation in local space)
    MatrixD3 &rotateYL(Dbl angle);             // rotate matrix by its y vector (y-axis rotation in local space)
    MatrixD3 &rotateZL(Dbl angle);             // rotate matrix by its z vector (z-axis rotation in local space)

    MatrixD3 &rotateXLOrthoNormalized(Dbl angle); // rotate matrix by its x vector (x-axis rotation in local space), this method is faster than 'rotateXL' however matrix must be orthogonal and normalized
    MatrixD3 &rotateYLOrthoNormalized(Dbl angle); // rotate matrix by its y vector (y-axis rotation in local space), this method is faster than 'rotateYL' however matrix must be orthogonal and normalized
    MatrixD3 &rotateZLOrthoNormalized(Dbl angle); // rotate matrix by its z vector (z-axis rotation in local space), this method is faster than 'rotateZL' however matrix must be orthogonal and normalized

    MatrixD3 &mirrorX();              // mirror matrix in X axis
    MatrixD3 &mirrorY();              // mirror matrix in Y axis
    MatrixD3 &mirrorZ();              // mirror matrix in Z axis
    MatrixD3 &mirror(C VecD &normal); // mirror matrix by   plane normal

    // set (set methods reset the full matrix)
    MatrixD3 &identity(); // set as identity
    MatrixD3 &zero();     // set all vectors to zero

    MatrixD3 &setScale(Dbl scale);     // set as scaled identity
    MatrixD3 &setScale(C VecD &scale); // set as scaled identity

    MatrixD3 &setRotateX(Dbl angle);              // set as   x-rotated identity
    MatrixD3 &setRotateY(Dbl angle);              // set as   y-rotated identity
    MatrixD3 &setRotateZ(Dbl angle);              // set as   z-rotated identity
    MatrixD3 &setRotateXY(Dbl x, Dbl y);          // set as x-y-rotated identity, works the same as 'setRotateX(x).rotateY(y)' but faster
    MatrixD3 &setRotate(C VecD &axis, Dbl angle); // set as     rotated by vector identity, 'axis' must be normalized
#if EE_PRIVATE
    MatrixD3 &setRotateCosSin(C VecD &axis, Dbl cos, Dbl sin); // set as rotated by vector identity, 'axis' must be normalized
#endif

    MatrixD3 &setOrient(DIR_ENUM dir);                                            // set as orientation from DIR_ENUM
    MatrixD3 &setRight(C VecD &right);                                            // set as x='right'         and calculate correct y,z, 'right'            must be normalized
    MatrixD3 &setRight(C VecD &right, C VecD &up);                                // set as x='right', y='up' and calculate correct z  , 'right' 'up'       must be normalized
    MatrixD3 &setUp(C VecD &up);                                                  // set as y='up'            and calculate correct x,z, 'up'               must be normalized
    MatrixD3 &setDir(C VecD &dir);                                                // set as z='dir'           and calculate correct x,y, 'dir'              must be normalized
    MatrixD3 &setDir(C VecD &dir, C VecD &up);                                    // set as z='dir', y='up'   and calculate correct x  , 'dir' 'up'         must be normalized
    MatrixD3 &setDir(C VecD &dir, C VecD &up, C VecD &right);                     // set as z='dir', y='up', x='right'                 , 'dir' 'up' 'right' must be normalized
    MatrixD3 &setRotation(C VecD &dir_from, C VecD &dir_to);                      // set as matrix which rotates 'dir_from' to 'dir_to',                                              'dir_from' 'dir_to' must be normalized
    MatrixD3 &setRotation(C VecD &dir_from, C VecD &dir_to, Dbl blend);           // set as matrix which rotates 'dir_from' to 'dir_to', using blend value                          , 'dir_from' 'dir_to' must be normalized
    MatrixD3 &setRotation(C VecD &dir_from, C VecD &dir_to, Dbl blend, Dbl roll); // set as matrix which rotates 'dir_from' to 'dir_to', using blend value and additional roll angle, 'dir_from' 'dir_to' must be normalized

    // get
    Dbl determinant() C;
    Bool mirrored() C { return determinant() < 0; } // if matrix is mirrored ('x' axis is on the other/left side)

    VecD scale() C;   // get each    axis scale
    VecD scale2() C;  // get each    axis scale squared
    Dbl avgScale() C; // get average axis scale
    Dbl maxScale() C; // get maximum axis scale

    VecD angles() C;                                      // get rotation angles, this allows to reconstruct the matrix using "setRotateZ(angle.z).rotateX(angle.x).rotateY(angle.y)" or faster by using "setRotateZ(angle.z).rotateXY(angle.x, angle.y)"
    VecD axis(Bool normalized = false) C;                 // get rotation axis                , if you know that the matrix is normalized then set 'normalized=true' for more performance
    Dbl angle(Bool normalized = false) C;                 // get rotation angle               , if you know that the matrix is normalized then set 'normalized=true' for more performance
    Dbl axisAngle(VecD &axis, Bool normalized = false) C; // get rotation axis and angle      , if you know that the matrix is normalized then set 'normalized=true' for more performance
    VecD axisAngle(Bool normalized = false) C;            // get rotation axis scaled by angle, if you know that the matrix is normalized then set 'normalized=true' for more performance
    Dbl angleY(Bool normalized = false) C;                // get rotation angle along Y axis  , if you know that the matrix is normalized then set 'normalized=true' for more performance, this is the same as "axisAngle(normalized).y" but faster

    Str asText(Int precision = INT_MAX) C { return S + "X: " + x.asText(precision) + ", Y: " + y.asText(precision) + ", Z:" + z.asText(precision); } // get text description

#if EE_PRIVATE
    // operations
    VecD2 convert(C VecD &src, Bool normalized = false) C;   // return converted 3D 'src' to 2D vector according to matrix x,y axes
    VecD convert(C VecD2 &src) C;                            // retrun converted 2D 'src' to 3D vector according to matrix x,y axes
    EdgeD2 convert(C EdgeD &src, Bool normalized = false) C; // return converted 3D 'src' to 2D edge   according to matrix x,y axes
    EdgeD convert(C EdgeD2 &src) C;                          // retrun converted 2D 'src' to 3D edge   according to matrix x,y axes
#endif

    // draw
    void draw(C VecD &pos, C Color &x_color = RED, C Color &y_color = GREEN, C Color &z_color = BLUE, Bool arrow = true) C; // draw axes, this can be optionally called outside of Render function, this relies on active object matrix which can be set using 'SetMatrix' function

    MatrixD3() {}
    MatrixD3(Dbl scale) { setScale(scale); }
    MatrixD3(C VecD &scale) { setScale(scale); }
    MatrixD3(C VecD &x, C VecD &y, C VecD &z) {
        T.x = x;
        T.y = y;
        T.z = z;
    }
    CONVERSION MatrixD3(C Matrix3 &m);
    CONVERSION MatrixD3(C MatrixD &m);
    CONVERSION MatrixD3(C Orient &o);
    CONVERSION MatrixD3(C OrientD &o);
    CONVERSION MatrixD3(C QuaternionD &q);
};
/******************************************************************************/
struct Matrix2P : Matrix2 // Matrix 3x2 (orientation + scale + position)
{
    Vec2 pos; // position

    Matrix2 &orn() { return T; }     // get reference to self as       'Matrix2'
    C Matrix2 &orn() C { return T; } // get reference to self as const 'Matrix2'

    // transform
    Matrix2P &operator*=(Flt f);
    Matrix2P &operator/=(Flt f);
    Matrix2P &operator*=(C Vec2 &v);
    Matrix2P &operator/=(C Vec2 &v);
    Matrix2P &operator+=(C Vec2 &v) {
        pos += v;
        return T;
    }
    Matrix2P &operator-=(C Vec2 &v) {
        pos -= v;
        return T;
    }
    Matrix2P &operator*=(C Matrix2 &m) { return mul(m); }
    Matrix2P &operator*=(C Matrix2P &m) { return mul(m); }
    Matrix2P &operator/=(C Matrix2 &m) { return div(m); }
    Matrix2P &operator/=(C Matrix2P &m) { return div(m); }
    Bool operator==(C Matrix2P &m) C;
    Bool operator!=(C Matrix2P &m) C;

    friend Matrix2P operator+(C Matrix2P &m, C Vec2 &v) { return Matrix2P(m) += v; } // get m+v
    friend Matrix2P operator-(C Matrix2P &m, C Vec2 &v) { return Matrix2P(m) -= v; } // get m-v
    friend Matrix2P operator*(C Matrix2P &a, C Matrix2 &b) {
        Matrix2P temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend Matrix2P operator*(C Matrix2P &a, C Matrix2P &b) {
        Matrix2P temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend Matrix2P operator/(C Matrix2P &a, C Matrix2 &b) {
        Matrix2P temp;
        a.div(b, temp);
        return temp;
    } // get a/b
    friend Matrix2P operator/(C Matrix2P &a, C Matrix2P &b) {
        Matrix2P temp;
        a.div(b, temp);
        return temp;
    } // get a/b
    friend Matrix2P operator~(C Matrix2P &m) {
        Matrix2P temp;
        m.inverse(temp);
        return temp;
    } // get inversed 'm'
    friend Matrix2P operator*(C Matrix2 &a, C Matrix2P &b) {
        Matrix2P temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend Matrix2P operator/(C Matrix2 &a, C Matrix2P &b) {
        Matrix2P temp;
        a.div(b, temp);
        return temp;
    } // get a/b

    void mul(C Matrix2 &matrix, Matrix2P &dest) C;  // multiply self by 'matrix' and store result in 'dest'
    void mul(C Matrix2P &matrix, Matrix2P &dest) C; // multiply self by 'matrix' and store result in 'dest'
    Matrix2P &mul(C Matrix2 &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'
    Matrix2P &mul(C Matrix2P &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'

    void div(C Matrix2 &matrix, Matrix2P &dest) C;  // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    void div(C Matrix2P &matrix, Matrix2P &dest) C; // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    Matrix2P &div(C Matrix2 &matrix) {
        div(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal
    Matrix2P &div(C Matrix2P &matrix) {
        div(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal

    void divNormalized(C Matrix2 &matrix, Matrix2P &dest) C;  // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
    void divNormalized(C Matrix2P &matrix, Matrix2P &dest) C; // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
    Matrix2P &divNormalized(C Matrix2 &matrix) {
        divNormalized(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized
    Matrix2P &divNormalized(C Matrix2P &matrix) {
        divNormalized(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized

    void inverse(Matrix2P &dest, Bool normalized = false) C; // inverse self to 'dest', if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal
    Matrix2P &inverse(Bool normalized = false) {
        inverse(T, normalized);
        return T;
    } // inverse self          , if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal

    Matrix2P &normalize() {
        super::normalize();
        return T;
    } // normalize scale           , this sets the length of 'x' 'y' 'z' vectors to 1
    Matrix2P &normalize(Flt scale) {
        super::normalize(scale);
        return T;
    } // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale'
    Matrix2P &normalize(C Vec2 &scale) {
        super::normalize(scale);
        return T;
    } // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale.x' 'scale.y' 'scale.z'

    Matrix2P &move(Flt x, Flt y) {
        pos += Vec2(x, y);
        return T;
    } // move
    Matrix2P &move(C Vec2 &move) {
        pos += move;
        return T;
    } // move
    Matrix2P &moveBack(C Vec2 &move) {
        pos -= move;
        return T;
    } // move back

    Matrix2P &scale(Flt scale) {
        T *= scale;
        return T;
    } // scale
    Matrix2P &scale(C Vec2 &scale) {
        T *= scale;
        return T;
    }                                // scale
    Matrix2P &scaleL(C Vec2 &scale); // scale in local space
    Matrix2P &scaleOrn(Flt scale) {
        super::scale(scale);
        return T;
    } // scale orientation only
    Matrix2P &scaleOrn(C Vec2 &scale) {
        super::scale(scale);
        return T;
    } // scale orientation only
    Matrix2P &scaleOrnL(C Vec2 &scale) {
        super::scaleL(scale);
        return T;
    } // scale orientation only in local space

    Matrix2P &rotate(Flt angle); // rotate
    Matrix2P &rotateL(Flt angle) {
        super::rotate(angle);
        return T;
    } // rotate in local space

    // set (set methods reset the full matrix)
    Matrix2P &identity(); // set as identity
    Matrix2P &zero();     // set all vectors to zero

    Matrix2P &setPos(Flt x, Flt y);                    // set as positioned identity
    Matrix2P &setPos(C Vec2 &pos);                     // set as positioned identity
    Matrix2P &setScale(Flt scale);                     // set as scaled     identity
    Matrix2P &setScale(C Vec2 &scale);                 // set as scaled     identity
    Matrix2P &setPosScale(C Vec2 &pos, Flt scale);     // set as positioned & scaled identity
    Matrix2P &setPosScale(C Vec2 &pos, C Vec2 &scale); // set as positioned & scaled identity
    Matrix2P &setScalePos(Flt scale, C Vec2 &pos);     // set as scaled & positioned identity
    Matrix2P &setScalePos(C Vec2 &scale, C Vec2 &pos); // set as scaled & positioned identity

    Matrix2P &setRotate(Flt angle); // set as rotated identity

    // get
    Vec2 scale() C { return super::scale(); } // get each axis scale

    Matrix2P() {}
    Matrix2P(Flt scale) { setScale(scale); }
    Matrix2P(C Vec2 &pos) { setPos(pos); }
    Matrix2P(C Vec2 &pos, Flt scale) { setPosScale(pos, scale); }
    Matrix2P(Flt scale, C Vec2 &pos) { setScalePos(scale, pos); }
    Matrix2P(C Vec2 &scale, C Vec2 &pos) { setScalePos(scale, pos); }
    Matrix2P(C Matrix2 &orn, C Vec2 &pos) {
        T.orn() = orn;
        T.pos = pos;
    }
    Matrix2P(C Vec2 &pos, C Matrix2 &orn) {
        T.orn() = orn;
        T.pos = pos;
    }
};
/******************************************************************************/
struct Matrix : Matrix3 // Matrix 4x3 (orientation + scale + position)
{
    Vec pos; // position

    Matrix3 &orn() { return T; }     // get reference to self as       'Matrix3'
    C Matrix3 &orn() C { return T; } // get reference to self as const 'Matrix3'

    // transform
    Matrix &operator*=(Flt f);
    Matrix &operator/=(Flt f);
    Matrix &operator*=(C Vec &v);
    Matrix &operator/=(C Vec &v);
    Matrix &operator+=(C Vec &v) {
        pos += v;
        return T;
    }
    Matrix &operator-=(C Vec &v) {
        pos -= v;
        return T;
    }
    Matrix &operator+=(C Matrix &m);
    Matrix &operator-=(C Matrix &m);
    Matrix &operator*=(C Matrix3 &m) { return mul(m); }
    Matrix &operator*=(C Matrix &m) { return mul(m); }
    Matrix &operator*=(C MatrixM &m) { return mul(m); }
    Matrix &operator/=(C Matrix3 &m) { return div(m); }
    Matrix &operator/=(C Matrix &m) { return div(m); }
    Matrix &operator/=(C MatrixM &m) { return div(m); }
    Matrix &operator*=(C RevMatrix &m) { return mul(m); }
    Bool operator==(C Matrix &m) C;
    Bool operator!=(C Matrix &m) C;

    friend Matrix operator+(C Matrix &m, C Vec &v) { return Matrix(m) += v; } // get m+v
    friend Matrix operator-(C Matrix &m, C Vec &v) { return Matrix(m) -= v; } // get m-v
    friend Matrix operator*(C Matrix &a, C Matrix3 &b) {
        Matrix temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend Matrix operator*(C Matrix &a, C Matrix &b) {
        Matrix temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend Matrix operator*(C Matrix &a, C RevMatrix &b) {
        Matrix temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend Matrix operator/(C Matrix &a, C Matrix3 &b) {
        Matrix temp;
        a.div(b, temp);
        return temp;
    } // get a/b
    friend Matrix operator/(C Matrix &a, C Matrix &b) {
        Matrix temp;
        a.div(b, temp);
        return temp;
    } // get a/b
    friend Matrix operator~(C Matrix &m) {
        Matrix temp;
        m.inverse(temp);
        return temp;
    } // get inversed 'm'
    friend Matrix operator*(C Matrix3 &a, C Matrix &b) {
        Matrix temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend Matrix operator/(C Matrix3 &a, C Matrix &b) {
        Matrix temp;
        a.div(b, temp);
        return temp;
    } // get a/b

    void mul(C Matrix3 &matrix, Matrix &dest) C;   // multiply self by 'matrix' and store result in 'dest'
    void mul(C Matrix &matrix, Matrix &dest) C;    // multiply self by 'matrix' and store result in 'dest'
    void mul(C MatrixM &matrix, Matrix &dest) C;   // multiply self by 'matrix' and store result in 'dest'
    void mul(C MatrixM &matrix, MatrixM &dest) C;  // multiply self by 'matrix' and store result in 'dest'
    void mul(C Matrix &matrix, Matrix4 &dest) C;   // multiply self by 'matrix' and store result in 'dest'
    void mul(C Matrix4 &matrix, Matrix4 &dest) C;  // multiply self by 'matrix' and store result in 'dest'
    void mul(C RevMatrix &matrix, Matrix &dest) C; // multiply self by 'matrix' and store result in 'dest'
    Matrix &mul(C Matrix3 &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'
    Matrix &mul(C Matrix &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'
    Matrix &mul(C MatrixM &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'
    Matrix &mul(C RevMatrix &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'

    void mulTimes(Int n, C Matrix &matrix, Matrix &dest) C;    // multiply self by 'matrix' 'n'-times and store result in 'dest'
    void mulTimes(Int n, C RevMatrix &matrix, Matrix &dest) C; // multiply self by 'matrix' 'n'-times and store result in 'dest'
    Matrix &mulTimes(Int n, C Matrix &matrix) {
        mulTimes(n, matrix, T);
        return T;
    } // multiply self by 'matrix' 'n'-times
    Matrix &mulTimes(Int n, C RevMatrix &matrix) {
        mulTimes(n, matrix, T);
        return T;
    } // multiply self by 'matrix' 'n'-times

    void div(C Matrix3 &matrix, Matrix &dest) C;  // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    void div(C Matrix &matrix, Matrix &dest) C;   // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    void div(C MatrixM &matrix, Matrix &dest) C;  // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    void div(C MatrixM &matrix, MatrixM &dest) C; // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    Matrix &div(C Matrix3 &matrix) {
        div(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal
    Matrix &div(C Matrix &matrix) {
        div(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal
    Matrix &div(C MatrixM &matrix) {
        div(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal

    void divNormalized(C Matrix3 &matrix, Matrix &dest) C; // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
    void divNormalized(C Matrix &matrix, Matrix &dest) C;  // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
    void divNormalized(C MatrixM &matrix, Matrix &dest) C; // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
    Matrix &divNormalized(C Matrix3 &matrix) {
        divNormalized(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized
    Matrix &divNormalized(C Matrix &matrix) {
        divNormalized(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized
    Matrix &divNormalized(C MatrixM &matrix) {
        divNormalized(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized

    void inverse(Matrix &dest, Bool normalized = false) C; // inverse self to 'dest', if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal
    Matrix &inverse(Bool normalized = false) {
        inverse(T, normalized);
        return T;
    } // inverse self          , if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal

    void inverseNonOrthogonal(Matrix &dest) C; // inverse self to 'dest', this method is slower than 'inverse' however it properly handles non-orthogonal matrixes
    Matrix &inverseNonOrthogonal() {
        inverseNonOrthogonal(T);
        return T;
    } // inverse self          , this method is slower than 'inverse' however it properly handles non-orthogonal matrixes

    Matrix &normalize() {
        super::normalize();
        return T;
    } // normalize scale           , this sets the length of 'x' 'y' 'z' vectors to 1
    Matrix &normalize(Flt scale) {
        super::normalize(scale);
        return T;
    } // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale'
    Matrix &normalize(C Vec &scale) {
        super::normalize(scale);
        return T;
    } // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale.x' 'scale.y' 'scale.z'

    Matrix &move(Flt x, Flt y, Flt z) {
        pos += Vec(x, y, z);
        return T;
    } // move
    Matrix &move(C Vec2 &move) {
        pos.xy += move;
        return T;
    } // move
    Matrix &move(C Vec &move) {
        pos += move;
        return T;
    } // move
    Matrix &moveBack(C Vec &move) {
        pos -= move;
        return T;
    } // move back

    Matrix &anchor(C Vec &anchor); // set 'pos' of this matrix so that 'anchor' transformed by this matrix will remain the same - "anchor*T==anchor"

    Matrix &scale(Flt scale) {
        T *= scale;
        return T;
    } // scale
    Matrix &scale(C Vec &scale) {
        T *= scale;
        return T;
    }                             // scale
    Matrix &scaleL(C Vec &scale); // scale in local space
    Matrix &scaleOrn(Flt scale) {
        super::scale(scale);
        return T;
    } // scale orientation only
    Matrix &scaleOrn(C Vec &scale) {
        super::scale(scale);
        return T;
    } // scale orientation only
    Matrix &scaleOrnL(C Vec &scale) {
        super::scaleL(scale);
        return T;
    }                                          // scale orientation only in local space
    Matrix &scale(C Vec &dir, Flt scale);      // scale along       'dir' direction by 'scale' value, 'dir' must be normalized
    Matrix &scalePlane(C Vec &nrm, Flt scale); // scale along plane of 'nrm' normal by 'scale' value, 'nrm' must be normalized

    Matrix &rotateX(Flt angle);             // rotate by x axis
    Matrix &rotateY(Flt angle);             // rotate by y axis
    Matrix &rotateZ(Flt angle);             // rotate by z axis
    Matrix &rotateXY(Flt x, Flt y);         // rotate by x axis and then by y axis, works the same as "rotateX(x).rotateY(y)" but faster
    Matrix &rotate(C Vec &axis, Flt angle); // rotate by vector, 'axis' must be normalized
    Matrix &rotateXL(Flt angle) {
        super::rotateXL(angle);
        return T;
    } // rotate matrix by its x vector (x-axis rotation in local space)
    Matrix &rotateYL(Flt angle) {
        super::rotateYL(angle);
        return T;
    } // rotate matrix by its y vector (y-axis rotation in local space)
    Matrix &rotateZL(Flt angle) {
        super::rotateZL(angle);
        return T;
    } // rotate matrix by its z vector (z-axis rotation in local space)

    Matrix &mirrorX();              // mirror matrix in X axis
    Matrix &mirrorY();              // mirror matrix in Y axis
    Matrix &mirrorZ();              // mirror matrix in Z axis
    Matrix &mirror(C Plane &plane); // mirror matrix by   plane

    Matrix &swapYZ(); // swap Y and Z components of every vector

    // set (set methods reset the full matrix)
    Matrix &identity();          // set as identity
    Matrix &identity(Flt blend); // set as identity, this method is similar to 'identity()' however it does not perform full reset of the matrix. Instead, smooth reset is applied depending on 'blend' value (0=no reset, 1=full reset)
    Matrix &zero();              // set all vectors to zero
#if EE_PRIVATE
    Matrix &keep01(Flt blend); // same as "identity(1-blend)", assumes 0<=blend<=1
#endif

    Matrix &setPos(Flt x, Flt y, Flt z);           // set as positioned identity
    Matrix &setPos(C Vec2 &pos);                   // set as positioned identity
    Matrix &setPos(C Vec &pos);                    // set as positioned identity
    Matrix &setScale(Flt scale);                   // set as scaled     identity
    Matrix &setScale(C Vec &scale);                // set as scaled     identity
    Matrix &setPosScale(C Vec &pos, Flt scale);    // set as positioned & scaled identity
    Matrix &setPosScale(C Vec &pos, C Vec &scale); // set as positioned & scaled identity
    Matrix &setScalePos(Flt scale, C Vec &pos);    // set as scaled & positioned identity
    Matrix &setScalePos(C Vec &scale, C Vec &pos); // set as scaled & positioned identity

    Matrix &setRotateX(Flt angle);                                                  // set as   x-rotated identity
    Matrix &setRotateY(Flt angle);                                                  // set as   y-rotated identity
    Matrix &setRotateZ(Flt angle);                                                  // set as   z-rotated identity
    Matrix &setRotateXY(Flt x, Flt y);                                              // set as x-y-rotated identity, works the same as setRotateX(x).rotateY(y) but faster
    Matrix &setRotate(C Vec &axis, Flt angle);                                      // set as     rotated by vector identity, 'axis' must be normalized
    Matrix &setRotation(C Vec &pos, C Vec &dir_from, C Vec &dir_to, Flt blend = 1); // set as matrix which rotates 'dir_from' to 'dir_to', using blend value, 'dir_from dir_to' must be normalized

    Matrix &setPosOrient(C Vec &pos, DIR_ENUM dir);                     // set as positioned orientation from DIR_ENUM
    Matrix &setPosRight(C Vec &pos, C Vec &right);                      // set as pos='pos', x='right'       and calculate correct y,z, 'right'            must be normalized
    Matrix &setPosUp(C Vec &pos, C Vec &up);                            // set as pos='pos', y='up'          and calculate correct x,z, 'up'               must be normalized
    Matrix &setPosDir(C Vec &pos, C Vec &dir);                          // set as pos='pos', z='dir'         and calculate correct x,y, 'dir'              must be normalized
    Matrix &setPosDir(C Vec &pos, C Vec &dir, C Vec &up);               // set as pos='pos', z='dir', y='up' and calculate correct x  , 'dir' 'up'         must be normalized
    Matrix &setPosDir(C Vec &pos, C Vec &dir, C Vec &up, C Vec &right); // set as pos='pos', z='dir', y='up', x='right'               , 'dir' 'up' 'right' must be normalized

    Matrix &setPosTerrainOrient(C Vec &pos, DIR_ENUM dir); // set as orientation from DIR_ENUM to be used for drawing spherical terrain heightmaps

    Matrix &set(C Box &src, C Box &dest); // set as matrix that transforms 'src' to 'dest' (src*m=dest)
    Matrix &setNormalizeX(C Box &box);    // set as matrix that (box*m).w()         =1
    Matrix &setNormalizeY(C Box &box);    // set as matrix that (box*m).h()         =1
    Matrix &setNormalizeZ(C Box &box);    // set as matrix that (box*m).d()         =1
    Matrix &setNormalize(C Box &box);     // set as matrix that (box*m).size().max()=1

    // get
    Vec scale() C { return super::scale(); } // get each axis scale

    Str asText(Int precision = INT_MAX) C { return super::asText(precision) + ", Pos: " + pos.asText(precision); } // get text description

    // operations
#if EE_PRIVATE
    Vec2 convert(C Vec &src, Bool normalized = false) C;   // return converted 3D 'src' to 2D vector according to matrix x,y axes and position
    Vec convert(C Vec2 &src) C;                            // return converted 2D 'src' to 3D vector according to matrix x,y axes and position
    Edge2 convert(C Edge &src, Bool normalized = false) C; // return converted 3D 'src' to 2D edge   according to matrix x,y axes and position
    Edge convert(C Edge2 &src) C;                          // return converted 2D 'src' to 3D edge   according to matrix x,y axes and position
#endif

    Matrix &setTransformAtPos(C Vec &pos, C Matrix3 &matrix); // set as transformation at position
    Matrix &setTransformAtPos(C Vec &pos, C Matrix &matrix);  // set as transformation at position
    Matrix &transformAtPos(C Vec &pos, C Matrix3 &matrix);    //        transform      at position
    Matrix &transformAtPos(C Vec &pos, C Matrix &matrix);     //        transform      at position

    // draw
    void draw(C Color &x_color = RED, C Color &y_color = GREEN, C Color &z_color = BLUE, Bool arrow = true) C { super::draw(pos, x_color, y_color, z_color, arrow); } // draw axes, this can be optionally called outside of Render function, this relies on active object matrix which can be set using 'SetMatrix' function

    Matrix() {}
    Matrix(Flt scale) { setScale(scale); }
    Matrix(C Vec &pos) { setPos(pos); }
    Matrix(C Vec &pos, Flt scale) { setPosScale(pos, scale); }
    Matrix(Flt scale, C Vec &pos) { setScalePos(scale, pos); }
    Matrix(C Vec &scale, C Vec &pos) { setScalePos(scale, pos); }
    Matrix(C Matrix3 &orn, C Vec &pos) {
        T.orn() = orn;
        T.pos = pos;
    }
    Matrix(C Vec &pos, C Matrix3 &orn) {
        T.orn() = orn;
        T.pos = pos;
    }
    CONVERSION Matrix(C Matrix3 &m);
    CONVERSION Matrix(C MatrixD3 &m);
    CONVERSION Matrix(C MatrixM &m);
    CONVERSION Matrix(C MatrixD &m);
    CONVERSION Matrix(C Matrix4 &m);
    CONVERSION Matrix(C OrientP &o);
    CONVERSION Matrix(C OrientM &o);
    CONVERSION Matrix(C OrientPD &o);
};
extern Matrix const MatrixIdentity; // identity
#if EE_PRIVATE

#define MAX_MATRIX 256 // max available Object Matrixes (256 Matrix * 3 Vec4 = 768 Vec4's)

struct GpuVelocity {
    Vec lin;
    Flt padd0;
    Vec ang;
    Flt padd1;

    void set(C Vec &lin, C Vec &ang) {
        T.lin = lin;
        T.ang = ang;
    }
};

extern MatrixM ObjMatrix, // object matrix
    CamMatrix,            // camera, this is always set, even when drawing shadows
    CamMatrixInv,         // camera inversed = ~CamMatrix
    CamMatrixInvPrev,     // camera inversed = ~CamMatrix (from previous frame)
    EyeMatrix[2],         // 'ActiveCam. matrix     ' adjusted for both eyes 0=left, 1=right
    EyeMatrixPrev[2];     // 'ActiveCam._matrix_prev' adjusted for both eyes 0=left, 1=right
extern GpuMatrix *ViewMatrix, *ViewMatrixPrev;
#endif
/******************************************************************************/
struct MatrixM : Matrix3 // Matrix 4x3 (orientation + scale + position, mixed precision, uses Flt for orientation+scale and Dbl for position)
{
    VecD pos; // position

    Matrix3 &orn() { return T; }     // get reference to self as       'Matrix3'
    C Matrix3 &orn() C { return T; } // get reference to self as const 'Matrix3'

    // transform
    MatrixM &operator*=(Flt f);
    MatrixM &operator/=(Flt f);
    MatrixM &operator*=(C Vec &v);
    MatrixM &operator/=(C Vec &v);
    MatrixM &operator+=(C Vec &v) {
        pos += v;
        return T;
    }
    MatrixM &operator-=(C Vec &v) {
        pos -= v;
        return T;
    }
    MatrixM &operator+=(C VecD &v) {
        pos += v;
        return T;
    }
    MatrixM &operator-=(C VecD &v) {
        pos -= v;
        return T;
    }
    MatrixM &operator+=(C MatrixM &m);
    MatrixM &operator-=(C MatrixM &m);
    MatrixM &operator*=(C Matrix3 &m) { return mul(m); }
    MatrixM &operator*=(C Matrix &m) { return mul(m); }
    MatrixM &operator*=(C MatrixM &m) { return mul(m); }
    MatrixM &operator/=(C Matrix3 &m) { return div(m); }
    MatrixM &operator/=(C Matrix &m) { return div(m); }
    MatrixM &operator/=(C MatrixM &m) { return div(m); }
    Bool operator==(C MatrixM &m) C;
    Bool operator!=(C MatrixM &m) C;

    friend MatrixM operator+(C MatrixM &m, C Vec &v) { return MatrixM(m) += v; }  // get m+v
    friend MatrixM operator-(C MatrixM &m, C Vec &v) { return MatrixM(m) -= v; }  // get m-v
    friend MatrixM operator+(C MatrixM &m, C VecD &v) { return MatrixM(m) += v; } // get m+v
    friend MatrixM operator-(C MatrixM &m, C VecD &v) { return MatrixM(m) -= v; } // get m-v
    friend MatrixM operator*(C MatrixM &a, C Matrix3 &b) {
        MatrixM temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend MatrixM operator*(C MatrixM &a, C Matrix &b) {
        MatrixM temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend MatrixM operator*(C MatrixM &a, C MatrixM &b) {
        MatrixM temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend MatrixM operator/(C MatrixM &a, C Matrix3 &b) {
        MatrixM temp;
        a.div(b, temp);
        return temp;
    } // get a/b
    friend MatrixM operator/(C MatrixM &a, C Matrix &b) {
        MatrixM temp;
        a.div(b, temp);
        return temp;
    } // get a/b
    friend MatrixM operator/(C MatrixM &a, C MatrixM &b) {
        MatrixM temp;
        a.div(b, temp);
        return temp;
    } // get a/b
    friend MatrixM operator~(C MatrixM &m) {
        MatrixM temp;
        m.inverse(temp);
        return temp;
    } // get inversed 'm'
    friend MatrixM operator*(C Matrix3 &a, C MatrixM &b) {
        MatrixM temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend MatrixM operator/(C Matrix3 &a, C MatrixM &b) {
        MatrixM temp;
        a.div(b, temp);
        return temp;
    } // get a/b
    friend MatrixM operator*(C Matrix &a, C MatrixM &b) {
        MatrixM temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend MatrixM operator/(C Matrix &a, C MatrixM &b) {
        MatrixM temp;
        a.div(b, temp);
        return temp;
    } // get a/b

    void mul(C MatrixM &matrix, Matrix &dest) C;  // multiply self by 'matrix' and store result in 'dest'
    void mul(C Matrix3 &matrix, MatrixM &dest) C; // multiply self by 'matrix' and store result in 'dest'
    void mul(C Matrix &matrix, MatrixM &dest) C;  // multiply self by 'matrix' and store result in 'dest'
    void mul(C MatrixM &matrix, MatrixM &dest) C; // multiply self by 'matrix' and store result in 'dest'
    MatrixM &mul(C Matrix3 &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'
    MatrixM &mul(C Matrix &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'
    MatrixM &mul(C MatrixM &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'

    void div(C MatrixM &matrix, Matrix &dest) C;  // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    void div(C Matrix3 &matrix, MatrixM &dest) C; // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    void div(C Matrix &matrix, MatrixM &dest) C;  // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    void div(C MatrixM &matrix, MatrixM &dest) C; // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    MatrixM &div(C Matrix3 &matrix) {
        div(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal
    MatrixM &div(C Matrix &matrix) {
        div(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal
    MatrixM &div(C MatrixM &matrix) {
        div(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal

    void divNormalized(C Matrix3 &matrix, MatrixM &dest) C; // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
    void divNormalized(C MatrixM &matrix, Matrix &dest) C;  // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
    void divNormalized(C Matrix &matrix, MatrixM &dest) C;  // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
    void divNormalized(C MatrixM &matrix, MatrixM &dest) C; // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
    MatrixM &divNormalized(C Matrix3 &matrix) {
        divNormalized(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized
    MatrixM &divNormalized(C Matrix &matrix) {
        divNormalized(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized
    MatrixM &divNormalized(C MatrixM &matrix) {
        divNormalized(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized

    void inverse(MatrixM &dest, Bool normalized = false) C; // inverse self to 'dest', if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal
    MatrixM &inverse(Bool normalized = false) {
        inverse(T, normalized);
        return T;
    } // inverse self          , if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal

    void inverseNonOrthogonal(MatrixM &dest) C; // inverse self to 'dest', this method is slower than 'inverse' however it properly handles non-orthogonal matrixes
    MatrixM &inverseNonOrthogonal() {
        inverseNonOrthogonal(T);
        return T;
    } // inverse self          , this method is slower than 'inverse' however it properly handles non-orthogonal matrixes

    MatrixM &normalize() {
        super::normalize();
        return T;
    } // normalize scale           , this sets the length of 'x' 'y' 'z' vectors to 1
    MatrixM &normalize(Flt scale) {
        super::normalize(scale);
        return T;
    } // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale'
    MatrixM &normalize(C Vec &scale) {
        super::normalize(scale);
        return T;
    } // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale.x' 'scale.y' 'scale.z'

    MatrixM &move(Dbl x, Dbl y, Dbl z) {
        pos += VecD(x, y, z);
        return T;
    } // move
    MatrixM &move(C VecD2 &move) {
        pos.xy += move;
        return T;
    } // move
    MatrixM &move(C VecD &move) {
        pos += move;
        return T;
    } // move
    MatrixM &moveBack(C VecD &move) {
        pos -= move;
        return T;
    } // move back

    MatrixM &anchor(C Vec &anchor);  // set 'pos' of this matrix so that 'anchor' transformed by this matrix will remain the same - "anchor*T==anchor"
    MatrixM &anchor(C VecD &anchor); // set 'pos' of this matrix so that 'anchor' transformed by this matrix will remain the same - "anchor*T==anchor"

    MatrixM &scale(Flt scale) {
        T *= scale;
        return T;
    } // scale
    MatrixM &scale(C Vec &scale) {
        T *= scale;
        return T;
    } // scale
    MatrixM &scaleOrn(Flt scale) {
        super::scale(scale);
        return T;
    } // scale orientation only
    MatrixM &scaleOrn(C Vec &scale) {
        super::scale(scale);
        return T;
    } // scale orientation only

    MatrixM &rotateX(Flt angle);             // rotate by x axis
    MatrixM &rotateY(Flt angle);             // rotate by y axis
    MatrixM &rotateZ(Flt angle);             // rotate by z axis
    MatrixM &rotateXY(Flt x, Flt y);         // rotate by x axis and then by y axis, works the same as "rotateX(x).rotateY(y)" but faster
    MatrixM &rotate(C Vec &axis, Flt angle); // rotate by vector, 'axis' must be normalized
    MatrixM &rotateXL(Flt angle) {
        super::rotateXL(angle);
        return T;
    } // rotate matrix by its x vector (x-axis rotation in local space)
    MatrixM &rotateYL(Flt angle) {
        super::rotateYL(angle);
        return T;
    } // rotate matrix by its y vector (y-axis rotation in local space)
    MatrixM &rotateZL(Flt angle) {
        super::rotateZL(angle);
        return T;
    } // rotate matrix by its z vector (z-axis rotation in local space)

    MatrixM &mirror(C PlaneM &plane); // mirror matrix by plane

    // get
    Vec scale() C { return super::scale(); } // get each axis scale

    Str asText(Int precision = INT_MAX) C { return super::asText(precision) + ", Pos: " + pos.asText(precision); } // get text description

    // set (set methods reset the full matrix)
    MatrixM &identity();          // set as identity
    MatrixM &identity(Flt blend); // set as identity, this method is similar to 'identity()' however it does not perform full reset of the matrix. Instead, smooth reset is applied depending on 'blend' value (0=no reset, 1=full reset)
    MatrixM &zero();              // set all vectors to zero
#if EE_PRIVATE
    MatrixM &keep01(Flt blend); // same as "identity(1-blend)", assumes 0<=blend<=1
#endif

    MatrixM &setPos(Dbl x, Dbl y, Dbl z);            // set as positioned identity
    MatrixM &setPos(C VecD2 &pos);                   // set as positioned identity
    MatrixM &setPos(C VecD &pos);                    // set as positioned identity
    MatrixM &setScale(Flt scale);                    // set as scaled     identity
    MatrixM &setScale(C Vec &scale);                 // set as scaled     identity
    MatrixM &setPosScale(C VecD &pos, Flt scale);    // set as positioned & scaled identity
    MatrixM &setPosScale(C VecD &pos, C Vec &scale); // set as positioned & scaled identity
    MatrixM &setScalePos(Flt scale, C VecD &pos);    // set as scaled & positioned identity
    MatrixM &setScalePos(C Vec &scale, C VecD &pos); // set as scaled & positioned identity

    MatrixM &setRotateX(Flt angle);             // set as   x-rotated identity
    MatrixM &setRotateY(Flt angle);             // set as   y-rotated identity
    MatrixM &setRotateZ(Flt angle);             // set as   z-rotated identity
    MatrixM &setRotateXY(Flt x, Flt y);         // set as x-y-rotated identity, works the same as setRotateX(x).rotateY(y) but faster
    MatrixM &setRotate(C Vec &axis, Flt angle); // set as     rotated by vector identity, 'axis' must be normalized

    MatrixM &setPosOrient(C VecD &pos, DIR_ENUM dir);                     // set as positioned orientation from DIR_ENUM
    MatrixM &setPosRight(C VecD &pos, C Vec &right);                      // set as pos='pos', x='right'       and calculate correct y,z, 'right'            must be normalized
    MatrixM &setPosUp(C VecD &pos, C Vec &up);                            // set as pos='pos', y='up'          and calculate correct x,z, 'up'               must be normalized
    MatrixM &setPosDir(C VecD &pos, C Vec &dir);                          // set as pos='pos', z='dir'         and calculate correct x,y, 'dir'              must be normalized
    MatrixM &setPosDir(C VecD &pos, C Vec &dir, C Vec &up);               // set as pos='pos', z='dir', y='up' and calculate correct x  , 'dir' 'up'         must be normalized
    MatrixM &setPosDir(C VecD &pos, C Vec &dir, C Vec &up, C Vec &right); // set as pos='pos', z='dir', y='up', x='right'               , 'dir' 'up' 'right' must be normalized

    MatrixM &setPosTerrainOrient(C VecD &pos, DIR_ENUM dir); // set as orientation from DIR_ENUM to be used for drawing spherical terrain heightmaps

    // operations
#if EE_PRIVATE
    Vec2 convert(C VecD &src, Bool normalized = false) C;   // return converted 3D 'src' to 2D vector according to matrix x,y axes and position
    VecD convert(C Vec2 &src) C;                            // return converted 2D 'src' to 3D vector according to matrix x,y axes and position
    Edge2 convert(C EdgeD &src, Bool normalized = false) C; // return converted 3D 'src' to 2D edge   according to matrix x,y axes and position
    EdgeD convert(C Edge2 &src) C;                          // return converted 2D 'src' to 3D edge   according to matrix x,y axes and position
#endif

    MatrixM &setTransformAtPos(C VecD &pos, C Matrix3 &matrix); // set as transformation at position
    MatrixM &setTransformAtPos(C VecD &pos, C MatrixM &matrix); // set as transformation at position
    MatrixM &transformAtPos(C VecD &pos, C Matrix3 &matrix);    //        transform      at position
    MatrixM &transformAtPos(C VecD &pos, C MatrixM &matrix);    //        transform      at position

    MatrixM() {}
    MatrixM(Flt scale) { setScale(scale); }
    MatrixM(C Vec &pos) { setPos(pos); }
    MatrixM(C VecD &pos) { setPos(pos); }
    MatrixM(C VecD &pos, Flt scale) { setPosScale(pos, scale); }
    MatrixM(Flt scale, C VecD &pos) { setScalePos(scale, pos); }
    MatrixM(C Vec &scale, C VecD &pos) { setScalePos(scale, pos); }
    MatrixM(C Matrix3 &orn, C VecD &pos) {
        T.orn() = orn;
        T.pos = pos;
    }
    MatrixM(C VecD &pos, C Matrix3 &orn) {
        T.orn() = orn;
        T.pos = pos;
    }
    CONVERSION MatrixM(C Matrix3 &m);
    CONVERSION MatrixM(C Matrix &m);
    CONVERSION MatrixM(C MatrixD &m);
    CONVERSION MatrixM(C OrientP &o);
    CONVERSION MatrixM(C OrientM &o);
    CONVERSION MatrixM(C OrientPD &o);
};
extern MatrixM const MatrixMIdentity; // identity
/******************************************************************************/
struct MatrixD : MatrixD3 // Matrix 4x3 (orientation + scale + position, double precision)
{
    VecD pos; // position

    MatrixD3 &orn() { return T; }     // get reference to self as       'MatrixD3'
    C MatrixD3 &orn() C { return T; } // get reference to self as const 'MatrixD3'

    // transform
    MatrixD &operator*=(Dbl f);
    MatrixD &operator/=(Dbl f);
    MatrixD &operator*=(C VecD &v);
    MatrixD &operator/=(C VecD &v);
    MatrixD &operator+=(C VecD &v) {
        pos += v;
        return T;
    }
    MatrixD &operator-=(C VecD &v) {
        pos -= v;
        return T;
    }
    MatrixD &operator+=(C MatrixD &m);
    MatrixD &operator-=(C MatrixD &m);
    MatrixD &operator*=(C MatrixD3 &m) { return mul(m); }
    MatrixD &operator*=(C MatrixD &m) { return mul(m); }
    MatrixD &operator/=(C MatrixD3 &m) { return div(m); }
    MatrixD &operator/=(C MatrixD &m) { return div(m); }
    Bool operator==(C MatrixD &m) C;
    Bool operator!=(C MatrixD &m) C;

    friend MatrixD operator*(C MatrixD &a, C MatrixD3 &b) {
        MatrixD temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend MatrixD operator*(C MatrixD &a, C MatrixD &b) {
        MatrixD temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend MatrixD operator*(C MatrixD &a, C MatrixM &b) {
        MatrixD temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend MatrixD operator/(C MatrixD &a, C MatrixD3 &b) {
        MatrixD temp;
        a.div(b, temp);
        return temp;
    } // get a/b
    friend MatrixD operator/(C MatrixD &a, C MatrixD &b) {
        MatrixD temp;
        a.div(b, temp);
        return temp;
    } // get a/b
    friend MatrixD operator~(C MatrixD &m) {
        MatrixD temp;
        m.inverse(temp);
        return temp;
    } // get inversed 'm'
    friend MatrixD operator*(C MatrixD3 &a, C MatrixD &b) {
        MatrixD temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend MatrixD operator/(C MatrixD3 &a, C MatrixD &b) {
        MatrixD temp;
        a.div(b, temp);
        return temp;
    } // get a/b

    void mul(C MatrixD3 &matrix, MatrixD &dest) C; // multiply self by 'matrix' and store result in 'dest'
    void mul(C MatrixD &matrix, MatrixD &dest) C;  // multiply self by 'matrix' and store result in 'dest'
    void mul(C MatrixM &matrix, MatrixD &dest) C;  // multiply self by 'matrix' and store result in 'dest'
    MatrixD &mul(C MatrixD3 &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'
    MatrixD &mul(C MatrixD &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'
    MatrixD &mul(C MatrixM &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'

    void div(C MatrixD3 &matrix, MatrixD &dest) C; // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    void div(C MatrixD &matrix, MatrixD &dest) C;  // divide self by 'matrix' and store result in 'dest', this method assumes that matrixes are orthogonal
    MatrixD &div(C MatrixD3 &matrix) {
        div(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal
    MatrixD &div(C MatrixD &matrix) {
        div(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method assumes that matrixes are orthogonal

    void divNormalized(C MatrixD3 &matrix, MatrixD &dest) C; // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
    void divNormalized(C MatrixD &matrix, MatrixD &dest) C;  // divide self by 'matrix' and store result in 'dest', this method is faster than 'div' however 'matrix' must be normalized
    MatrixD &divNormalized(C MatrixD3 &matrix) {
        divNormalized(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized
    MatrixD &divNormalized(C MatrixD &matrix) {
        divNormalized(matrix, T);
        return T;
    } // divide self by 'matrix'                           , this method is faster than 'div' however 'matrix' must be normalized

    void inverse(MatrixD &dest, Bool normalized = false) C; // inverse self to 'dest', if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal
    MatrixD &inverse(Bool normalized = false) {
        inverse(T, normalized);
        return T;
    } // inverse self          , if you know that the matrix is normalized then set 'normalized=true' for more performance, this method assumes that matrix is orthogonal

    void inverseNonOrthogonal(MatrixD &dest) C; // inverse self to 'dest', this method is slower than 'inverse' however it properly handles non-orthogonal matrixes
    MatrixD &inverseNonOrthogonal() {
        inverseNonOrthogonal(T);
        return T;
    } // inverse self          , this method is slower than 'inverse' however it properly handles non-orthogonal matrixes

    MatrixD &normalize() {
        super::normalize();
        return T;
    } // normalize scale           , this sets the length of 'x' 'y' 'z' vectors to 1
    MatrixD &normalize(Dbl scale) {
        super::normalize(scale);
        return T;
    } // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale'
    MatrixD &normalize(C VecD &scale) {
        super::normalize(scale);
        return T;
    } // normalize scale to 'scale', this sets the length of 'x' 'y' 'z' vectors to 'scale.x' 'scale.y' 'scale.z'

    MatrixD &move(Dbl x, Dbl y, Dbl z) {
        pos += VecD(x, y, z);
        return T;
    } // move
    MatrixD &move(C VecD2 &move) {
        pos.xy += move;
        return T;
    } // move
    MatrixD &move(C VecD &move) {
        pos += move;
        return T;
    } // move
    MatrixD &moveBack(C VecD &move) {
        pos -= move;
        return T;
    } // move back

    MatrixD &anchor(C VecD &anchor); // set 'pos' of this matrix so that 'anchor' transformed by this matrix will remain the same - "anchor*T==anchor"

    MatrixD &scale(Dbl scale) {
        T *= scale;
        return T;
    } // scale
    MatrixD &scale(C VecD &scale) {
        T *= scale;
        return T;
    } // scale
    MatrixD &scaleOrn(Dbl scale) {
        super::scale(scale);
        return T;
    } // scale orientation only
    MatrixD &scaleOrn(C VecD &scale) {
        super::scale(scale);
        return T;
    }                                            // scale orientation only
    MatrixD &scale(C VecD &dir, Dbl scale);      // scale along       'dir' direction by 'scale' value, 'dir' must be normalized
    MatrixD &scalePlane(C VecD &nrm, Dbl scale); // scale along plane of 'nrm' normal by 'scale' value, 'nrm' must be normalized

    MatrixD &rotateX(Dbl angle);              // rotate by x axis
    MatrixD &rotateY(Dbl angle);              // rotate by y axis
    MatrixD &rotateZ(Dbl angle);              // rotate by z axis
    MatrixD &rotateXY(Dbl x, Dbl y);          // rotate by x axis and then by y axis, works the same as "rotateX(x).rotateY(y)" but faster
    MatrixD &rotate(C VecD &axis, Dbl angle); // rotate by vector, 'axis' must be normalized
    MatrixD &rotateXL(Dbl angle) {
        super::rotateXL(angle);
        return T;
    } // rotate matrix by its x vector (x-axis rotation in local space)
    MatrixD &rotateYL(Dbl angle) {
        super::rotateYL(angle);
        return T;
    } // rotate matrix by its y vector (y-axis rotation in local space)
    MatrixD &rotateZL(Dbl angle) {
        super::rotateZL(angle);
        return T;
    } // rotate matrix by its z vector (z-axis rotation in local space)

    MatrixD &mirrorX();               // mirror matrix in X axis
    MatrixD &mirrorY();               // mirror matrix in Y axis
    MatrixD &mirrorZ();               // mirror matrix in Z axis
    MatrixD &mirror(C PlaneD &plane); // mirror matrix by   plane

    // set (set methods reset the full matrix)
    MatrixD &identity(); // set as identity
    MatrixD &zero();     // set all vectors to zero

    MatrixD &setPos(Dbl x, Dbl y, Dbl z);             // set as positioned identity
    MatrixD &setPos(C VecD2 &pos);                    // set as positioned identity
    MatrixD &setPos(C VecD &pos);                     // set as positioned identity
    MatrixD &setScale(Dbl scale);                     // set as scaled     identity
    MatrixD &setScale(C VecD &scale);                 // set as scaled     identity
    MatrixD &setPosScale(C VecD &pos, Dbl scale);     // set as positioned & scaled identity
    MatrixD &setPosScale(C VecD &pos, C VecD &scale); // set as positioned & scaled identity
    MatrixD &setScalePos(Dbl scale, C VecD &pos);     // set as scaled & positioned identity
    MatrixD &setScalePos(C VecD &scale, C VecD &pos); // set as scaled & positioned identity

    MatrixD &setRotateX(Dbl angle);              // set as   x-rotated identity
    MatrixD &setRotateY(Dbl angle);              // set as   y-rotated identity
    MatrixD &setRotateZ(Dbl angle);              // set as   z-rotated identity
    MatrixD &setRotateXY(Dbl x, Dbl y);          // set as x-y-rotated identity, works the same as setRotateX(x).rotateY(y) but faster
    MatrixD &setRotate(C VecD &axis, Dbl angle); // set as     rotated by vector identity, 'axis' must be normalized

    MatrixD &setPosOrient(C VecD &pos, DIR_ENUM dir);                        // set as positioned orientation from DIR_ENUM
    MatrixD &setPosRight(C VecD &pos, C VecD &right);                        // set as pos='pos', x='right'       and calculate correct y,z, 'right'            must be normalized
    MatrixD &setPosUp(C VecD &pos, C VecD &up);                              // set as pos='pos', y='up'          and calculate correct x,z, 'up'               must be normalized
    MatrixD &setPosDir(C VecD &pos, C VecD &dir);                            // set as pos='pos', z='dir'         and calculate correct x,y, 'dir'              must be normalized
    MatrixD &setPosDir(C VecD &pos, C VecD &dir, C VecD &up);                // set as pos='pos', z='dir', y='up' and calculate correct x  , 'dir' 'up'         must be normalized
    MatrixD &setPosDir(C VecD &pos, C VecD &dir, C VecD &up, C VecD &right); // set as pos='pos', z='dir', y='up', x='right'               , 'dir' 'up' 'right' must be normalized

    // get
    VecD scale() C { return super::scale(); } // get each axis scale

    Str asText(Int precision = INT_MAX) C { return super::asText(precision) + ", Pos: " + pos.asText(precision); } // get text description

    // operations
#if EE_PRIVATE
    VecD2 convert(C VecD &src, Bool normalized = false) C;   // return converted 3D 'src' to 2D vector according to matrix x,y axes and position
    VecD convert(C VecD2 &src) C;                            // return converted 2D 'src' to 3D vector according to matrix x,y axes and position
    EdgeD2 convert(C EdgeD &src, Bool normalized = false) C; // return converted 3D 'src' to 2D edge   according to matrix x,y axes and position
    EdgeD convert(C EdgeD2 &src) C;                          // return converted 2D 'src' to 3D edge   according to matrix x,y axes and position
#endif

    MatrixD &setTransformAtPos(C VecD &pos, C MatrixD3 &matrix); // set as transformation at position
    MatrixD &setTransformAtPos(C VecD &pos, C MatrixD &matrix);  // set as transformation at position
    MatrixD &transformAtPos(C VecD &pos, C MatrixD3 &matrix);    //        transform      at position
    MatrixD &transformAtPos(C VecD &pos, C MatrixD &matrix);     //        transform      at position

    // draw
    void draw(C Color &x_color = RED, C Color &y_color = GREEN, C Color &z_color = BLUE, Bool arrow = true) C { super::draw(pos, x_color, y_color, z_color, arrow); } // draw axes, this can be optionally called outside of Render function, this relies on active object matrix which can be set using 'SetMatrix' function

    MatrixD() {}
    MatrixD(Dbl scale) { setScale(scale); }
    MatrixD(C VecD &pos) { setPos(pos); }
    MatrixD(C VecD &pos, Dbl scale) { setPosScale(pos, scale); }
    MatrixD(Dbl scale, C VecD &pos) { setScalePos(scale, pos); }
    MatrixD(C VecD &scale, C VecD &pos) { setScalePos(scale, pos); }
    MatrixD(C MatrixD3 &orn, C VecD &pos) {
        T.orn() = orn;
        T.pos = pos;
    }
    MatrixD(C VecD &pos, C MatrixD3 &orn) {
        T.orn() = orn;
        T.pos = pos;
    }
    CONVERSION MatrixD(C MatrixD3 &m);
    CONVERSION MatrixD(C Matrix &m);
    CONVERSION MatrixD(C MatrixM &m);
    CONVERSION MatrixD(C OrientP &o);
    CONVERSION MatrixD(C OrientM &o);
    CONVERSION MatrixD(C OrientPD &o);
};
/******************************************************************************/
struct Matrix4 // Matrix 4x4
{
    Vec4 x, y, z, pos;

#if EE_PRIVATE
    Flt determinant() C;
#endif

    // transform
    Matrix4 &operator*=(C Matrix4 &m) { return mul(m); }

    friend Matrix4 operator*(C Matrix4 &a, C Matrix4 &b) {
        Matrix4 temp;
        a.mul(b, temp);
        return temp;
    } // get a*b
    friend Matrix4 operator*(C Matrix &a, C Matrix4 &b) {
        Matrix4 temp;
        a.mul(b, temp);
        return temp;
    } // get a*b

    Matrix4 &inverse();   // inverse   self
    Matrix4 &transpose(); // transpose self

    void mul(C Matrix4 &matrix, Matrix4 &dest) C; // multiply self by 'matrix' and store result in 'dest'
    Matrix4 &mul(C Matrix4 &matrix) {
        mul(matrix, T);
        return T;
    } // multiply self by 'matrix'

#if EE_PRIVATE
    void offsetX(Flt dx);
    void offsetY(Flt dy);
#endif

    // set (set methods reset the full matrix)
    Matrix4 &identity(); // set as identity
    Matrix4 &zero();     // set all vectors to zero

    Matrix4() {}
    CONVERSION Matrix4(C Matrix3 &m);
    CONVERSION Matrix4(C Matrix &m);
};
#if EE_PRIVATE
extern Matrix4 ProjMatrix; // Projection Matrix
extern Flt ProjMatrixEyeOffset[2];
#endif
/******************************************************************************/
#if EE_PRIVATE
struct GpuMatrix // GPU Matrix (transposed Matrix)
{
    Flt xx, yx, zx, _x,
        xy, yy, zy, _y,
        xz, yz, zz, _z;

    GpuMatrix &fromMul(C Matrix &a, C Matrix &b);           // set from "a*b"
    GpuMatrix &fromMul(C Matrix &a, C MatrixM &b);          // set from "a*b"
    GpuMatrix &fromMul(C MatrixM &a, C Matrix &b) = delete; // set from "a*b"
    GpuMatrix &fromMul(C MatrixM &a, C MatrixM &b);         // set from "a*b"

    GpuMatrix() {}
    CONVERSION GpuMatrix(C Matrix &m);
    CONVERSION GpuMatrix(C MatrixM &m);
};
#endif
struct RevMatrix3 : Matrix3 // Reverse Matrix
{
};
struct RevMatrix : Matrix // Reverse Matrix
{
    RevMatrix3 &orn() { return (RevMatrix3 &)T; }     // get reference to self as       'RevMatrix3'
    C RevMatrix3 &orn() C { return (RevMatrix3 &)T; } // get reference to self as const 'RevMatrix3'
};
/******************************************************************************/
#if EE_PRIVATE
extern Int Matrixes; // number of active matrixes
#endif
/******************************************************************************/
Bool Equal(C Matrix3 &a, C Matrix3 &b, Flt eps = EPS);
Bool Equal(C MatrixD3 &a, C MatrixD3 &b, Dbl eps = EPSD);
Bool Equal(C Matrix &a, C Matrix &b, Flt eps = EPS, Flt pos_eps = EPS);
Bool Equal(C MatrixM &a, C MatrixM &b, Flt eps = EPS, Dbl pos_eps = EPSD);
Bool Equal(C MatrixD &a, C MatrixD &b, Dbl eps = EPSD, Dbl pos_eps = EPSD);
Bool Equal(C Matrix4 &a, C Matrix4 &b, Flt eps = EPS);

void GetTransform(Matrix3 &transform, C Orient &start, C Orient &result);      // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
void GetTransform(Matrix3 &transform, C Matrix3 &start, C Matrix3 &result);    // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
void GetTransform(MatrixD3 &transform, C MatrixD3 &start, C MatrixD3 &result); // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
void GetTransform(Matrix &transform, C Matrix &start, C Matrix &result);       // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
void GetTransform(MatrixM &transform, C MatrixM &start, C MatrixM &result);    // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
void GetTransform(MatrixD &transform, C MatrixD &start, C MatrixD &result);    // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
Matrix3 GetTransform(C Orient &start, C Orient &result);                       // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
Matrix3 GetTransform(C Matrix3 &start, C Matrix3 &result);                     // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
MatrixD3 GetTransform(C MatrixD3 &start, C MatrixD3 &result);                  // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
Matrix GetTransform(C Matrix &start, C Matrix &result);                        // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
MatrixM GetTransform(C MatrixM &start, C MatrixM &result);                     // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
MatrixD GetTransform(C MatrixD &start, C MatrixD &result);                     // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"

inline void GetTransform(RevMatrix &transform, C Matrix &start, C Matrix &result) { result.div(start, transform); }                     // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result"
inline void GetTransformNormalized(RevMatrix &transform, C Matrix &start, C Matrix &result) { result.divNormalized(start, transform); } // get 'transform' matrix that transforms 'start' to 'result' according to following formula "start*transform=result", this function assumes that 'start' and 'result' are normalized

void GetDelta(Vec &angle, C Matrix3 &prev, C Matrix3 &cur);                              // get              angle axis delta from 'prev' and 'cur' matrixes !! matrixes DON'T have to be normalized !!
void GetDelta(Vec &pos, Vec &angle, C Matrix &prev, C Matrix &cur);                      // get position and angle axis delta from 'prev' and 'cur' matrixes !! matrixes DON'T have to be normalized !!
void GetDelta(Vec &pos, Vec &angle, C MatrixM &prev, C MatrixM &cur);                    // get position and angle axis delta from 'prev' and 'cur' matrixes !! matrixes DON'T have to be normalized !!
void GetDelta(Vec &pos, Vec &angle, C Vec &prev2_pos, C Matrix &prev, C Matrix &cur);    // get position and angle axis delta from 'prev' and 'cur' matrixes !! matrixes DON'T have to be normalized !! 'prev2_pos'=position one step before 'prev', used to calculate more smooth 'pos' delta
void GetDelta(Vec &pos, Vec &angle, C VecD &prev2_pos, C MatrixM &prev, C MatrixM &cur); // get position and angle axis delta from 'prev' and 'cur' matrixes !! matrixes DON'T have to be normalized !! 'prev2_pos'=position one step before 'prev', used to calculate more smooth 'pos' delta

void GetVel(Vec &vel, Vec &ang_vel, C Matrix &prev, C Matrix &cur, Flt dt = Time.d());                      // get linear velocity and angular velocity from 'prev' and 'cur' matrixes using 'dt' time delta !! matrixes DON'T have to be normalized !!
void GetVel(Vec &vel, Vec &ang_vel, C MatrixM &prev, C MatrixM &cur, Flt dt = Time.d());                    // get linear velocity and angular velocity from 'prev' and 'cur' matrixes using 'dt' time delta !! matrixes DON'T have to be normalized !!
void GetVel(Vec &vel, Vec &ang_vel, C Vec &prev2_pos, C Matrix &prev, C Matrix &cur, Flt dt = Time.d());    // get linear velocity and angular velocity from 'prev' and 'cur' matrixes using 'dt' time delta !! matrixes DON'T have to be normalized !! 'prev2_pos'=position one step before 'prev', used to calculate more smooth 'vel' velocity
void GetVel(Vec &vel, Vec &ang_vel, C VecD &prev2_pos, C MatrixM &prev, C MatrixM &cur, Flt dt = Time.d()); // get linear velocity and angular velocity from 'prev' and 'cur' matrixes using 'dt' time delta !! matrixes DON'T have to be normalized !! 'prev2_pos'=position one step before 'prev', used to calculate more smooth 'vel' velocity

Flt GetLodDist2(C Vec &lod_center);                    // calculate squared distance from 'lod_center'                         to active camera, returned value can be used as parameter for 'Mesh.getDrawLod' methods
Flt GetLodDist2(C Vec &lod_center, C Matrix3 &matrix); // calculate squared distance from 'lod_center' transformed by 'matrix' to active camera, returned value can be used as parameter for 'Mesh.getDrawLod' methods
Flt GetLodDist2(C Vec &lod_center, C Matrix &matrix);  // calculate squared distance from 'lod_center' transformed by 'matrix' to active camera, returned value can be used as parameter for 'Mesh.getDrawLod' methods
Flt GetLodDist2(C Vec &lod_center, C MatrixM &matrix); // calculate squared distance from 'lod_center' transformed by 'matrix' to active camera, returned value can be used as parameter for 'Mesh.getDrawLod' methods

#if EE_PRIVATE
void InitMatrix();

void SetMatrixCount(Int num = 1); // set number of used matrixes and velocities
void SetFurVelCount(Int num = 1); // set number of used fur velocities

void SetFastMatrix();                  // set object matrix to 'MatrixIdentity'
void SetFastMatrix(C Matrix &matrix);  // set object matrix
void SetFastMatrix(C MatrixM &matrix); // set object matrix

void SetFastMatrixPrev();                  // set object matrix to 'MatrixIdentity'
void SetFastMatrixPrev(C Matrix &matrix);  // set object matrix
void SetFastMatrixPrev(C MatrixM &matrix); // set object matrix

void SetOneMatrix();                                          // set object matrix to 'MatrixIdentity' and number of used matrixes to 1
void SetOneMatrix(C Matrix &matrix);                          // set object matrix                     and number of used matrixes to 1
void SetOneMatrix(C MatrixM &matrix);                         // set object matrix                     and number of used matrixes to 1
void SetOneMatrix(C Matrix &matrix, C Matrix &matrix_prev);   // set object matrix                     and number of used matrixes to 1
void SetOneMatrix(C MatrixM &matrix, C MatrixM &matrix_prev); // set object matrix                     and number of used matrixes to 1

void SetOneMatrixAndPrev(); // set object matrix and matrix_prev to 'MatrixIdentity', and number of used matrixes to 1

void SetVelFur(C Matrix3 &view_matrix, C Vec &vel); // set velocity for fur effect

void SetProjMatrix();
void SetProjMatrix(Flt proj_offset);
#endif

void SetMatrix();                                          // set active object rendering matrix to 'MatrixIdentity'
void SetMatrix(C Matrix &matrix);                          // set active object rendering matrix
void SetMatrix(C MatrixM &matrix);                         // set active object rendering matrix
void SetMatrix(C Matrix &matrix, C Matrix &matrix_prev);   // set active object rendering matrix for current frame and that was used in previous frame
void SetMatrix(C MatrixM &matrix, C MatrixM &matrix_prev); // set active object rendering matrix for current frame and that was used in previous frame
/******************************************************************************/
