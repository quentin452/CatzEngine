/******************************************************************************/
#include "../../stdafx.h"
namespace EE {
/******************************************************************************

   Order of planes is set based on performance tests.

/******************************************************************************/
FrustumClass Frustum,
    FrustumMain; // it's set as the #1 Main Frustum, #2 Mirrored Frustum (for Reflections), but NOT set when rendering shadows.
/******************************************************************************/
void FrustumClass::setExtraPlane(C PlaneM &plane, Bool chs) {
    extra |= PLANE;
    extra_plane = plane;
    if (chs)
        extra_plane.normal.chs();
    extra_plane_n_abs = Abs(extra_plane.normal);
}
void FrustumClass::setExtraBall(C BallM &ball) {
    extra |= BALL;
    extra_ball = ball;
    extra_ball_r2 = Sqr(ball.r);
}
/******************************************************************************/
void FrustumClass::set(Flt range, C Vec2 &fov, C MatrixM &camera) {
    T.range = range;
    T.matrix = camera;

    // set planes
    if (persp = FovPerspective(D._view_active.fov_mode)) {
        Vec2 fov_sin, fov_cos;

        if (VR.active() && D._allow_stereo) {
            eye_dist_2 = D.eyeDistance_2();
            fov_tan.y = D._view_active.fov_tan.y;
            fov_sin.y = D._view_active.fov_sin.y;
            fov_cos.y = D._view_active.fov_cos.y;

            //                   proj_center=desc.DefaultEyeFov[0].LeftTan/(desc.DefaultEyeFov[0].LeftTan+desc.DefaultEyeFov[0].RightTan);
            //                       fov_tan=                         0.5f*(desc.DefaultEyeFov[0].LeftTan+desc.DefaultEyeFov[0].RightTan);
            // desc.DefaultEyeFov[0].LeftTan=proj_center*fov_tan*2;
            Flt proj_center = ProjMatrixEyeOffset[0] * 0.5f + 0.5f;
            fov_tan.x = proj_center * D._view_active.fov_tan.x * 2; // outer tangent, this is correct, because when *0.95 multiplier was applied, then objects were culled near the edge of the screen
            CosSin(fov_cos.x, fov_sin.x, Atan(fov_tan.x));
        } else {
            eye_dist_2 = 0;
            fov_tan = D._view_active.fov_tan;
            fov_sin = D._view_active.fov_sin;
            fov_cos = D._view_active.fov_cos;
        }
        fov_cos_inv = 1.0f / fov_cos; // calculate inverse, because later we'll be able to use "*fov_cos_inv" instead of "/fov_cos", as multiplication is faster than division

        Vec view_quad_max(fov_tan.x * D._view_active.from, fov_tan.y * D._view_active.from, D._view_active.from);
        view_quad_max_dist = view_quad_max.length();

        plane[DIR_RIGHT].normal.set(fov_cos.x, 0, -fov_sin.x);
        plane[DIR_RIGHT].pos.set(eye_dist_2, 0, 0); // right
        plane[DIR_LEFT].normal.set(-fov_cos.x, 0, -fov_sin.x);
        plane[DIR_LEFT].pos.set(-eye_dist_2, 0, 0); // left
        plane[DIR_UP].normal.set(0, fov_cos.y, -fov_sin.y);
        plane[DIR_UP].pos.zero(); // up
        plane[DIR_DOWN].normal.set(0, -fov_cos.y, -fov_sin.y);
        plane[DIR_DOWN].pos.zero(); // down
        plane[DIR_FORWARD].normal.set(0, 0, 1);
        plane[DIR_FORWARD].pos.set(0, 0, range); // front
        plane[DIR_BACK].normal.set(0, 0, -1);
        plane[DIR_BACK].pos.set(0, 0, D._view_active.from); // back
    } else {
        view_quad_max_dist = 0;
        eye_dist_2 = 0;
        size.xy = fov;
        size.z = range * 0.5f;           // set as half because we're extending both ways
        matrix.pos += matrix.z * size.z; // set 'pos' in the center, ortho mode was designed to always have the position in the center so we can use fast frustum culling for Z axis the same way as for XY axes

        plane[DIR_RIGHT].normal.set(1, 0, 0);
        plane[DIR_RIGHT].pos.set(size.x, 0, 0); // right
        plane[DIR_LEFT].normal.set(-1, 0, 0);
        plane[DIR_LEFT].pos.set(-size.x, 0, 0); // left
        plane[DIR_UP].normal.set(0, 1, 0);
        plane[DIR_UP].pos.set(0, size.y, 0); // up
        plane[DIR_DOWN].normal.set(0, -1, 0);
        plane[DIR_DOWN].pos.set(0, -size.y, 0); // down
        plane[DIR_FORWARD].normal.set(0, 0, 1);
        plane[DIR_FORWARD].pos.set(0, 0, size.z); // front
        plane[DIR_BACK].normal.set(0, 0, -1);
        plane[DIR_BACK].pos.set(0, 0, -size.z); // back
    }

    REPA(plane) {
        plane[i].pos *= matrix;
        plane[i].normal *= matrix.orn();
        plane_n_abs[i] = Abs(plane[i].normal);
    }

    extra = 0;
    if (Renderer.mirror())
        setExtraPlane(Renderer._mirror_plane, true);
    /*else
    if(Water.draw_plane_surface)
    {
       Flt density=Water.density+Water.density_add;
       if( density>0)
       {
          planes++;
          plane[6]=Water.plane;
          plane[6].normal.chs();
          if(density<1)
          {
             // underwater opacity = Sat(pow(1-Water.density, x)-Water.density_add);
             // pow(1-Water.density, x)-Water.density_add = eps
             // pow(1-Water.density, x) = eps+Water.density_add
             // a**c=b <=> loga(b)=c
             // (1-Water.density)**x=eps+Water.density_add <=> log 1-Water.density (eps+Water.density_add)=x

             plane[6]+=plane[6].normal*Max(0,Log(0.015f+Water.density_add,1-Water.density));
          }
          plane[6]+=plane[6].normal*Water.wave_scale;
       }
    }*/

    // points and edges
    if (persp) {
        // points
        {
            Flt z = range,
                x = fov_tan.x * z,
                y = fov_tan.y * z;
            point[0] = matrix.pos;
            // set in clock-wise order, do not change the order as it is used in 'ShadowMap'
            point[1].set(-x, y, z);
            point[2].set(x, y, z);
            point[3].set(x, -y, z);
            point[4].set(-x, -y, z);
            Transform(point + 1, matrix, 4);
        }
        // edges
        {
            edge[0].set(0, 1);
            edge[1].set(0, 2);
            edge[2].set(0, 3);
            edge[3].set(0, 4);

            edge[4].set(1, 2);
            edge[5].set(2, 3);
            edge[6].set(3, 4);
            edge[7].set(4, 1);
        }
        points = 5;
        edges = 8;
    } else {
        // points
        {
            Flt x = size.x,
                y = size.y,
                z = size.z;
            // set in clock-wise order, do not change the order as it is used in 'ShadowMap' and 'ConnectedPoints'
            point[0].set(-x, y, -z);
            point[1].set(x, y, -z);
            point[2].set(x, -y, -z);
            point[3].set(-x, -y, -z);

            point[4].set(-x, y, z);
            point[5].set(x, y, z);
            point[6].set(x, -y, z);
            point[7].set(-x, -y, z);

            Transform(point, matrix, 8);
        }
        // edges
        {
            edge[0].set(0, 4);
            edge[1].set(1, 5);
            edge[2].set(2, 6);
            edge[3].set(3, 7);

            edge[4].set(0, 1);
            edge[5].set(1, 2);
            edge[6].set(2, 3);
            edge[7].set(3, 0);

            edge[8].set(4, 5);
            edge[9].set(5, 6);
            edge[10].set(6, 7);
            edge[11].set(7, 4);
        }
        points = 8;
        edges = 12;
    }
}
void FrustumClass::set() {
    set(D._view_active.range, D._view_active.fov, CamMatrix);
    FrustumMain = T;
}
/******************************************************************************/
void FrustumClass::from(C BoxD &box) {
    // set planes
    plane[DIR_RIGHT].normal.set(1, 0, 0);
    plane[DIR_RIGHT].pos.set(box.max.x, 0, 0); // right
    plane[DIR_LEFT].normal.set(-1, 0, 0);
    plane[DIR_LEFT].pos.set(box.min.x, 0, 0); // left
    plane[DIR_UP].normal.set(0, 1, 0);
    plane[DIR_UP].pos.set(0, box.max.y, 0); // up
    plane[DIR_DOWN].normal.set(0, -1, 0);
    plane[DIR_DOWN].pos.set(0, box.min.y, 0); // down
    plane[DIR_FORWARD].normal.set(0, 0, 1);
    plane[DIR_FORWARD].pos.set(0, 0, box.max.z); // front
    plane[DIR_BACK].normal.set(0, 0, -1);
    plane[DIR_BACK].pos.set(0, 0, box.min.z); // back

    // set helpers
    extra = 0;
    view_quad_max_dist = 0;
    eye_dist_2 = 0;
    persp = false;
    size = box.size() * 0.5;
    matrix.setPos(box.center());
    REPA(plane)
    plane_n_abs[i] = Abs(plane[i].normal);

    // points
    {
        point[0].set(box.min.x, box.max.y, box.min.z);
        point[1].set(box.max.x, box.max.y, box.min.z);
        point[2].set(box.max.x, box.min.y, box.min.z);
        point[3].set(box.min.x, box.min.y, box.min.z);

        point[4].set(box.min.x, box.max.y, box.max.z);
        point[5].set(box.max.x, box.max.y, box.max.z);
        point[6].set(box.max.x, box.min.y, box.max.z);
        point[7].set(box.min.x, box.min.y, box.max.z);
    }
    // edges
    {
        edge[0].set(0, 4);
        edge[1].set(1, 5);
        edge[2].set(2, 6);
        edge[3].set(3, 7);

        edge[4].set(0, 1);
        edge[5].set(1, 2);
        edge[6].set(2, 3);
        edge[7].set(3, 0);

        edge[8].set(4, 5);
        edge[9].set(5, 6);
        edge[10].set(6, 7);
        edge[11].set(7, 4);
    }
    points = 8;
    edges = 12;
}
/******************************************************************************/
void FrustumClass::from(C PyramidM &pyramid) // used for cone lights
{
    // set planes
    Vec2 d(pyramid.scale, 1);
    d.normalize();

    /*plane[DIR_RIGHT  ].normal.set(    D._view_active.fov_cos.x, 0, -D._view_active.fov_sin.x); plane[DIR_RIGHT  ].pos.zero(                          ); // right
      plane[DIR_LEFT   ].normal.set(   -D._view_active.fov_cos.x, 0, -D._view_active.fov_sin.x); plane[DIR_LEFT   ].pos.zero(                          ); // left
      plane[DIR_UP     ].normal.set(0,  D._view_active.fov_cos.y,    -D._view_active.fov_sin.y); plane[DIR_UP     ].pos.zero(                          ); // up
      plane[DIR_DOWN   ].normal.set(0, -D._view_active.fov_cos.y,    -D._view_active.fov_sin.y); plane[DIR_DOWN   ].pos.zero(                          ); // down
      plane[DIR_FORWARD].normal.set(0,                         0,     1                       ); plane[DIR_FORWARD].pos.set (0, 0, D._view_active.range); // front
      plane[DIR_BACK   ].normal.set(0,                         0,    -1                       ); plane[DIR_BACK   ].pos.set (0, 0, D._view_active.from ); // back*/

    plane[DIR_FORWARD].normal = pyramid.dir;
    plane[DIR_FORWARD].pos = pyramid.pos + pyramid.dir * pyramid.h; // front
    plane[DIR_BACK].normal = -pyramid.dir;
    plane[DIR_BACK].pos = pyramid.pos; // back

    plane[DIR_RIGHT].normal.set(d.y, 0, -d.x);
    plane[DIR_RIGHT].pos.zero(); // right
    plane[DIR_LEFT].normal.set(-d.y, 0, -d.x);
    plane[DIR_LEFT].pos.zero(); // left
    plane[DIR_UP].normal.set(0, d.y, -d.x);
    plane[DIR_UP].pos.zero(); // up
    plane[DIR_DOWN].normal.set(0, -d.y, -d.x);
    plane[DIR_DOWN].pos.zero(); // down

    matrix = pyramid;
    DIR_ENUM dir[] = {DIR_RIGHT, DIR_LEFT, DIR_UP, DIR_DOWN};
    REPA(dir) {
        plane[dir[i]].pos *= matrix;
        plane[dir[i]].normal *= matrix.orn();
    }

    // set helpers
    eye_dist_2 = 0;
    view_quad_max_dist = 0;
    extra = 0;
    if (Renderer.mirror())
        setExtraPlane(Renderer._mirror_plane, true);
    persp = true;
    fov_tan = pyramid.scale;
    Vec2 fov_cos = d.y;
    fov_cos_inv = 1.0f / fov_cos;
    range = pyramid.h;
    REPA(plane)
    plane_n_abs[i] = Abs(plane[i].normal);

    // points
    {
        point[0] = matrix.pos;
        point[1] = matrix.pos + (matrix.z + (-matrix.x + matrix.y) * pyramid.scale) * pyramid.h;
        point[2] = matrix.pos + (matrix.z + (matrix.x + matrix.y) * pyramid.scale) * pyramid.h;
        point[3] = matrix.pos + (matrix.z + (matrix.x - matrix.y) * pyramid.scale) * pyramid.h;
        point[4] = matrix.pos + (matrix.z + (-matrix.x - matrix.y) * pyramid.scale) * pyramid.h;
    }
    // edges
    {
        edge[0].set(0, 1);
        edge[1].set(0, 2);
        edge[2].set(0, 3);
        edge[3].set(0, 4);

        edge[4].set(1, 2);
        edge[5].set(2, 3);
        edge[6].set(3, 4);
        edge[7].set(4, 1);
    }
    points = 5;
    edges = 8;
}
/******************************************************************************/
Dbl FrustumClass::volume() C // !! Warning: this ignores 'extra' !!
{
    if (persp) {
        return (fov_tan.x * fov_tan.y * (4.0f / 3)) * Cube(Dbl(range));
    } else {
        return Dbl(size.x * 8) * size.y * size.z; // 2*2*2
    }
}
/******************************************************************************/
Bool FrustumClass::operator()(C Vec &point) C {
    Vec pos = point - matrix.pos; // no need for 'VecD'
    if (persp) {
        Flt z = Dot(pos, matrix.z);
        if (z < 0 || z > range)
            return false;
        Flt x = Dot(pos, matrix.x), bx = fov_tan.x * z + eye_dist_2;
        if (Abs(x) > bx)
            return false;
        Flt y = Dot(pos, matrix.y), by = fov_tan.y * z;
        if (Abs(y) > by)
            return false;

        if (extra) {
            if (extraPlane() && Dist(point, extra_plane) > 0)
                return false;
            if (extraBall() && Dist2(point, extra_ball.pos) > extra_ball_r2)
                return false;
        }
    } else {
        Flt y = Dot(pos, matrix.y), by = size.y;
        if (Abs(y) > by)
            return false;
        Flt x = Dot(pos, matrix.x), bx = size.x;
        if (Abs(x) > bx)
            return false;
        Flt z = Dot(pos, matrix.z), bz = size.z;
        if (Abs(z) > bz)
            return false;
    }
    return true;
}
Bool FrustumClass::operator()(C VecD &point) C {
    Vec pos = point - matrix.pos; // no need for 'VecD'
    if (persp) {
        Flt z = Dot(pos, matrix.z);
        if (z < 0 || z > range)
            return false;
        Flt x = Dot(pos, matrix.x), bx = fov_tan.x * z + eye_dist_2;
        if (Abs(x) > bx)
            return false;
        Flt y = Dot(pos, matrix.y), by = fov_tan.y * z;
        if (Abs(y) > by)
            return false;

        if (extra) {
            if (extraPlane() && Dist(point, extra_plane) > 0)
                return false;
            if (extraBall() && Dist2(point, extra_ball.pos) > extra_ball_r2)
                return false;
        }
    } else {
        Flt y = Dot(pos, matrix.y), by = size.y;
        if (Abs(y) > by)
            return false;
        Flt x = Dot(pos, matrix.x), bx = size.x;
        if (Abs(x) > bx)
            return false;
        Flt z = Dot(pos, matrix.z), bz = size.z;
        if (Abs(z) > bz)
            return false;
    }
    return true;
}
/******************************************************************************/
Bool FrustumClass::operator()(C Ball &ball) C {
    Vec pos = ball.pos - matrix.pos; // no need for 'VecD'
    if (persp) {
        Flt z = Dot(pos, matrix.z);
        if (z < -ball.r || z > range + ball.r)
            return false;
        MAX(z, 0);
        Flt x = Dot(pos, matrix.x), bx = fov_tan.x * z + ball.r * fov_cos_inv.x + eye_dist_2;
        if (Abs(x) > bx)
            return false;
        Flt y = Dot(pos, matrix.y), by = fov_tan.y * z + ball.r * fov_cos_inv.y;
        if (Abs(y) > by)
            return false;

        if (extra) {
            if (extraPlane() && Dist(ball, extra_plane) > 0)
                return false;
            if (extraBall() && !Cuts(ball, extra_ball))
                return false;
        }
    } else {
        Flt y = Dot(pos, matrix.y), by = size.y + ball.r;
        if (Abs(y) > by)
            return false;
        Flt x = Dot(pos, matrix.x), bx = size.x + ball.r;
        if (Abs(x) > bx)
            return false;
        Flt z = Dot(pos, matrix.z), bz = size.z + ball.r;
        if (Abs(z) > bz)
            return false;
    }
    return true;
}
Bool FrustumClass::operator()(C BallM &ball) C {
    Vec pos = ball.pos - matrix.pos; // no need for 'VecD'
    if (persp) {
        Flt z = Dot(pos, matrix.z);
        if (z < -ball.r || z > range + ball.r)
            return false;
        MAX(z, 0);
        Flt x = Dot(pos, matrix.x), bx = fov_tan.x * z + ball.r * fov_cos_inv.x + eye_dist_2;
        if (Abs(x) > bx)
            return false;
        Flt y = Dot(pos, matrix.y), by = fov_tan.y * z + ball.r * fov_cos_inv.y;
        if (Abs(y) > by)
            return false;

        if (extra) {
            if (extraPlane() && Dist(ball, extra_plane) > 0)
                return false;
            if (extraBall() && !Cuts(ball, extra_ball))
                return false;
        }
    } else {
        Flt y = Dot(pos, matrix.y), by = size.y + ball.r;
        if (Abs(y) > by)
            return false;
        Flt x = Dot(pos, matrix.x), bx = size.x + ball.r;
        if (Abs(x) > bx)
            return false;
        Flt z = Dot(pos, matrix.z), bz = size.z + ball.r;
        if (Abs(z) > bz)
            return false;
    }
    return true;
}
Bool FrustumClass::operator()(C Capsule &capsule) C {
    // if(capsule.isBall())return T(Ball(capsule)); // TODO: this is slower but more precise
    Vec up = capsule.pos - matrix.pos, // no need for 'VecD'
        down = up,
        d = capsule.up * capsule.innerHeightHalf();
    up += d;
    down -= d;
    if (persp) {
        Flt zu = Dot(up, matrix.z), zd = Dot(down, matrix.z);
        if ((zu < -capsule.r && zd < -capsule.r) || (zu > range + capsule.r && zd > range + capsule.r))
            return false;
        MAX(zu, 0);
        MAX(zd, 0);
        Flt xu = Dot(up, matrix.x), xd = Dot(down, matrix.x), bx = capsule.r * fov_cos_inv.x + eye_dist_2;
        if (Abs(xu) > bx + fov_tan.x * zu && Abs(xd) > bx + fov_tan.x * zd)
            return false;
        Flt yu = Dot(up, matrix.y), yd = Dot(down, matrix.y), by = capsule.r * fov_cos_inv.y;
        if (Abs(yu) > by + fov_tan.y * zu && Abs(yd) > by + fov_tan.y * zd)
            return false;

        if (extra) {
            if (extraPlane() && Dist(capsule, extra_plane) > 0)
                return false;
            if (extraBall() && !Cuts(extra_ball, capsule))
                return false;
        }
    } else {
        Flt yu = Dot(up, matrix.y), yd = Dot(down, matrix.y), by = size.y + capsule.r;
        if (Abs(yu) > by && Abs(yd) > by)
            return false;
        Flt xu = Dot(up, matrix.x), xd = Dot(down, matrix.x), bx = size.x + capsule.r;
        if (Abs(xu) > bx && Abs(xd) > bx)
            return false;
        Flt zu = Dot(up, matrix.z), zd = Dot(down, matrix.z), bz = size.z + capsule.r;
        if (Abs(zu) > bz && Abs(zd) > bz)
            return false;
    }
    return true;
}
Bool FrustumClass::operator()(C CapsuleM &capsule) C {
    // if(capsule.isBall())return T(BallM(capsule)); // TODO: this is slower but more precise
    Vec up = capsule.pos - matrix.pos, // no need for 'VecD'
        down = up,
        d = capsule.up * capsule.innerHeightHalf();
    up += d;
    down -= d;
    if (persp) {
        Flt zu = Dot(up, matrix.z), zd = Dot(down, matrix.z);
        if ((zu < -capsule.r && zd < -capsule.r) || (zu > range + capsule.r && zd > range + capsule.r))
            return false;
        MAX(zu, 0);
        MAX(zd, 0);
        Flt xu = Dot(up, matrix.x), xd = Dot(down, matrix.x), bx = capsule.r * fov_cos_inv.x + eye_dist_2;
        if (Abs(xu) > bx + fov_tan.x * zu && Abs(xd) > bx + fov_tan.x * zd)
            return false;
        Flt yu = Dot(up, matrix.y), yd = Dot(down, matrix.y), by = capsule.r * fov_cos_inv.y;
        if (Abs(yu) > by + fov_tan.y * zu && Abs(yd) > by + fov_tan.y * zd)
            return false;

        if (extra) {
            if (extraPlane() && Dist(capsule, extra_plane) > 0)
                return false;
            if (extraBall() && !Cuts(extra_ball, capsule))
                return false;
        }
    } else {
        Flt yu = Dot(up, matrix.y), yd = Dot(down, matrix.y), by = size.y + capsule.r;
        if (Abs(yu) > by && Abs(yd) > by)
            return false;
        Flt xu = Dot(up, matrix.x), xd = Dot(down, matrix.x), bx = size.x + capsule.r;
        if (Abs(xu) > bx && Abs(xd) > bx)
            return false;
        Flt zu = Dot(up, matrix.z), zd = Dot(down, matrix.z), bz = size.z + capsule.r;
        if (Abs(zu) > bz && Abs(zd) > bz)
            return false;
    }
    return true;
}
/******************************************************************************/
INLINE Flt BoxLength(C Vec &size, C Vec &dir) // box length along direction
{
    return Abs(dir.x) * size.x + Abs(dir.y) * size.y + Abs(dir.z) * size.z;
}
INLINE Flt BoxLengthAbs(C Vec &size, C Vec &dir_abs) // box length along direction, assuming that direction has absolute components (non-negative)
{
    return dir_abs.x * size.x + dir_abs.y * size.y + dir_abs.z * size.z;
}
INLINE Flt OBoxLength(C Vec &x, C Vec &y, C Vec &z, C Vec &dir) // obox length along direction
{
    return Abs(Dot(x, dir)) + Abs(Dot(y, dir)) + Abs(Dot(z, dir));
}
Bool FrustumClass::operator()(C Extent &ext) C {
    Vec pos = ext.pos - matrix.pos; // no need for 'VecD'
    if (persp) {
        Flt z = Dot(pos, matrix.z);
        if (z < 0 || z > range) {
            Flt bz = BoxLengthAbs(ext.ext, plane_n_abs[DIR_FORWARD]);
            if (z < -bz || z > range + bz)
                return false; // fb
            MAX(z, 0);
        }

        Flt x = Dot(pos, matrix.x), bx = fov_tan.x * z + eye_dist_2;
        if (Abs(x) > bx) {
            if (x > bx + BoxLengthAbs(ext.ext, plane_n_abs[DIR_RIGHT]) * fov_cos_inv.x)
                return false; // r
            if (x < -bx - BoxLengthAbs(ext.ext, plane_n_abs[DIR_LEFT]) * fov_cos_inv.x)
                return false; // l
        }

        Flt y = Dot(pos, matrix.y), by = fov_tan.y * z;
        if (Abs(y) > by) {
            if (y > by + BoxLengthAbs(ext.ext, plane_n_abs[DIR_UP]) * fov_cos_inv.y)
                return false; // u
            if (y < -by - BoxLengthAbs(ext.ext, plane_n_abs[DIR_DOWN]) * fov_cos_inv.y)
                return false; // d
        }

        if (extra) {
            if (extraPlane()) {
                Flt e = Dist(pos + matrix.pos, extra_plane);
                if (e > 0)
                    if (e > BoxLengthAbs(ext.ext, extra_plane_n_abs))
                        return false; // do a fast "e>0" check first to avoid calculating 'BoxLengthAbs' when unnecessary
            }
            if (extraBall() && Dist2(extra_ball.pos, ext) > extra_ball_r2)
                return false;
        }
    } else {
        Flt x = Abs(Dot(pos, matrix.x)), bx = T.size.x;
        if (x > bx)
            if (x > bx + BoxLengthAbs(ext.ext, plane_n_abs[DIR_RIGHT]))
                return false; // rl
        Flt y = Abs(Dot(pos, matrix.y)), by = T.size.y;
        if (y > by)
            if (y > by + BoxLengthAbs(ext.ext, plane_n_abs[DIR_UP]))
                return false; // ud
        Flt z = Abs(Dot(pos, matrix.z)), bz = T.size.z;
        if (z > bz)
            if (z > bz + BoxLengthAbs(ext.ext, plane_n_abs[DIR_FORWARD]))
                return false; // fb
    }
    return true;
}
Bool FrustumClass::operator()(C Extent &ext, C Matrix3 &matrix) C {
    Vec dx = ext.ext.x * matrix.x,
        dy = ext.ext.y * matrix.y,
        dz = ext.ext.z * matrix.z,
#if 0
       pos=ext.pos*matrix-T.matrix.pos; // no need for 'VecD'
#else // faster than above
        pos = ext.pos;
    pos *= matrix;
    pos -= T.matrix.pos; // #VecMulMatrix
#endif

    if (persp) {
        Flt z = Dot(pos, T.matrix.z);
        if (z < 0 || z > range) {
            Flt bz = OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal);
            if (z < -bz || z > range + bz)
                return false; // fb
            MAX(z, 0);
        }

        Flt x = Dot(pos, T.matrix.x), bx = fov_tan.x * z + eye_dist_2;
        if (Abs(x) > bx) {
            if (x > bx + OBoxLength(dx, dy, dz, plane[DIR_RIGHT].normal) * fov_cos_inv.x)
                return false; // r
            if (x < -bx - OBoxLength(dx, dy, dz, plane[DIR_LEFT].normal) * fov_cos_inv.x)
                return false; // l
        }

        Flt y = Dot(pos, T.matrix.y), by = fov_tan.y * z;
        if (Abs(y) > by) {
            if (y > by + OBoxLength(dx, dy, dz, plane[DIR_UP].normal) * fov_cos_inv.y)
                return false; // u
            if (y < -by - OBoxLength(dx, dy, dz, plane[DIR_DOWN].normal) * fov_cos_inv.y)
                return false; // d
        }

        if (extra) {
            if (extraPlane()) {
                Flt e = Dist(pos + T.matrix.pos, extra_plane);
                if (e > 0)
                    if (e > OBoxLength(dx, dy, dz, extra_plane.normal))
                        return false; // do a fast "e>0" check first to avoid calculating 'OBoxLengthAbs' when unnecessary
            }
            if (extraBall() && Dist2(extra_ball.pos, ext, matrix) > extra_ball_r2)
                return false;
        }
    } else {
        Flt x = Abs(Dot(pos, T.matrix.x)), bx = T.size.x;
        if (x > bx)
            if (x > bx + OBoxLength(dx, dy, dz, plane[DIR_RIGHT].normal))
                return false; // rl
        Flt y = Abs(Dot(pos, T.matrix.y)), by = T.size.y;
        if (y > by)
            if (y > by + OBoxLength(dx, dy, dz, plane[DIR_UP].normal))
                return false; // ud
        Flt z = Abs(Dot(pos, T.matrix.z)), bz = T.size.z;
        if (z > bz)
            if (z > bz + OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal))
                return false; // fb
    }
    return true;
}
Bool FrustumClass::operator()(C Extent &ext, C Matrix &matrix) C {
    Vec dx = ext.ext.x * matrix.x,
        dy = ext.ext.y * matrix.y,
        dz = ext.ext.z * matrix.z,
#if 0
       pos=ext.pos*matrix-T.matrix.pos; // no need for 'VecD'
#else // faster than above
        pos = ext.pos;
    pos *= matrix;
    pos -= T.matrix.pos; // #VecMulMatrix
#endif

    if (persp) {
        Flt z = Dot(pos, T.matrix.z);
        if (z < 0 || z > range) {
            Flt bz = OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal);
            if (z < -bz || z > range + bz)
                return false; // fb
            MAX(z, 0);
        }

        Flt x = Dot(pos, T.matrix.x), bx = fov_tan.x * z + eye_dist_2;
        if (Abs(x) > bx) {
            if (x > bx + OBoxLength(dx, dy, dz, plane[DIR_RIGHT].normal) * fov_cos_inv.x)
                return false; // r
            if (x < -bx - OBoxLength(dx, dy, dz, plane[DIR_LEFT].normal) * fov_cos_inv.x)
                return false; // l
        }

        Flt y = Dot(pos, T.matrix.y), by = fov_tan.y * z;
        if (Abs(y) > by) {
            if (y > by + OBoxLength(dx, dy, dz, plane[DIR_UP].normal) * fov_cos_inv.y)
                return false; // u
            if (y < -by - OBoxLength(dx, dy, dz, plane[DIR_DOWN].normal) * fov_cos_inv.y)
                return false; // d
        }

        if (extra) {
            if (extraPlane()) {
                Flt e = Dist(pos + T.matrix.pos, extra_plane);
                if (e > 0)
                    if (e > OBoxLength(dx, dy, dz, extra_plane.normal))
                        return false; // do a fast "e>0" check first to avoid calculating 'OBoxLengthAbs' when unnecessary
            }
            if (extraBall() && Dist2(extra_ball.pos, ext, matrix) > extra_ball_r2)
                return false;
        }
    } else {
        Flt x = Abs(Dot(pos, T.matrix.x)), bx = T.size.x;
        if (x > bx)
            if (x > bx + OBoxLength(dx, dy, dz, plane[DIR_RIGHT].normal))
                return false; // rl
        Flt y = Abs(Dot(pos, T.matrix.y)), by = T.size.y;
        if (y > by)
            if (y > by + OBoxLength(dx, dy, dz, plane[DIR_UP].normal))
                return false; // ud
        Flt z = Abs(Dot(pos, T.matrix.z)), bz = T.size.z;
        if (z > bz)
            if (z > bz + OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal))
                return false; // fb
    }
    return true;
}
Bool FrustumClass::operator()(C Extent &ext, C MatrixM &matrix) C {
    Vec dx = ext.ext.x * matrix.x,
        dy = ext.ext.y * matrix.y,
        dz = ext.ext.z * matrix.z,
        pos = ext.pos * matrix - T.matrix.pos; // no need for 'VecD' if all computations done before setting to 'Vec'

    if (persp) {
        Flt z = Dot(pos, T.matrix.z);
        if (z < 0 || z > range) {
            Flt bz = OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal);
            if (z < -bz || z > range + bz)
                return false; // fb
            MAX(z, 0);
        }

        Flt x = Dot(pos, T.matrix.x), bx = fov_tan.x * z + eye_dist_2;
        if (Abs(x) > bx) {
            if (x > bx + OBoxLength(dx, dy, dz, plane[DIR_RIGHT].normal) * fov_cos_inv.x)
                return false; // r
            if (x < -bx - OBoxLength(dx, dy, dz, plane[DIR_LEFT].normal) * fov_cos_inv.x)
                return false; // l
        }

        Flt y = Dot(pos, T.matrix.y), by = fov_tan.y * z;
        if (Abs(y) > by) {
            if (y > by + OBoxLength(dx, dy, dz, plane[DIR_UP].normal) * fov_cos_inv.y)
                return false; // u
            if (y < -by - OBoxLength(dx, dy, dz, plane[DIR_DOWN].normal) * fov_cos_inv.y)
                return false; // d
        }

        if (extra) {
            if (extraPlane()) {
                Flt e = Dist(pos + T.matrix.pos, extra_plane);
                if (e > 0)
                    if (e > OBoxLength(dx, dy, dz, extra_plane.normal))
                        return false; // do a fast "e>0" check first to avoid calculating 'OBoxLengthAbs' when unnecessary
            }
            if (extraBall() && Dist2(extra_ball.pos, ext, matrix) > extra_ball_r2)
                return false;
        }
    } else {
        Flt x = Abs(Dot(pos, T.matrix.x)), bx = T.size.x;
        if (x > bx)
            if (x > bx + OBoxLength(dx, dy, dz, plane[DIR_RIGHT].normal))
                return false; // rl
        Flt y = Abs(Dot(pos, T.matrix.y)), by = T.size.y;
        if (y > by)
            if (y > by + OBoxLength(dx, dy, dz, plane[DIR_UP].normal))
                return false; // ud
        Flt z = Abs(Dot(pos, T.matrix.z)), bz = T.size.z;
        if (z > bz)
            if (z > bz + OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal))
                return false; // fb
    }
    return true;
}
/******************************************************************************/
Bool FrustumClass::operator()(C Extent &ext, Bool &fully_inside) C {
    fully_inside = true;

    Vec pos = ext.pos - T.matrix.pos; // no need for 'VecD'
    if (persp) {
        Flt z = Dot(pos, T.matrix.z);
        // if( z<0 || z>range)
        {
            Flt bz = BoxLengthAbs(ext.ext, plane_n_abs[DIR_FORWARD]);
            if (z < bz) {
                if (z < -bz)
                    return false;
                fully_inside = false;
            } // b
            if (z > range - bz) {
                if (z > range + bz)
                    return false;
                fully_inside = false;
            } // f
            MAX(z, 0);
        }

        Flt x = Dot(pos, T.matrix.x), bx = fov_tan.x * z + eye_dist_2;
        // if(Abs(x)>bx)
        {
            Flt bxr = BoxLengthAbs(ext.ext, plane_n_abs[DIR_RIGHT]) * fov_cos_inv.x;
            if (x > bx - bxr) {
                if (x > bx + bxr)
                    return false;
                fully_inside = false;
            } // r
            Flt bxl = BoxLengthAbs(ext.ext, plane_n_abs[DIR_LEFT]) * fov_cos_inv.x;
            if (x < -bx + bxl) {
                if (x < -bx - bxl)
                    return false;
                fully_inside = false;
            } // l
        }

        Flt y = Dot(pos, T.matrix.y), by = fov_tan.y * z;
        // if(Abs(y)>by)
        {
            Flt bxu = BoxLengthAbs(ext.ext, plane_n_abs[DIR_UP]) * fov_cos_inv.y;
            if (y > by - bxu) {
                if (y > by + bxu)
                    return false;
                fully_inside = false;
            } // u
            Flt bxd = BoxLengthAbs(ext.ext, plane_n_abs[DIR_DOWN]) * fov_cos_inv.y;
            if (y < -by + bxd) {
                if (y < -by - bxd)
                    return false;
                fully_inside = false;
            } // d
        }

        if (extra) {
            if (extraPlane()) {
                Flt e = Dist(pos + T.matrix.pos, extra_plane), be = BoxLengthAbs(ext.ext, extra_plane_n_abs);
                if (e > -be) {
                    if (e > be)
                        return false;
                    fully_inside = false;
                }
            }
            if (extraBall()) {
                Vec d = extra_ball.pos - ext.pos;
                d.abs(); // no need for 'VecD'

                if (Dist2(Max(0, d.x - ext.ext.x),
                          Max(0, d.y - ext.ext.y),
                          Max(0, d.z - ext.ext.z)) > extra_ball_r2)
                    return false; // check minimum distance, if overlapping

                if (Dist2(d.x + ext.ext.x,
                          d.y + ext.ext.y,
                          d.z + ext.ext.z) > extra_ball_r2)
                    fully_inside = false; // check maximum distance, if fully inside
            }
        }
    } else {
        Flt x = Abs(Dot(pos, T.matrix.x)) - T.size.x, bx = BoxLengthAbs(ext.ext, plane_n_abs[DIR_RIGHT]);
        if (x > -bx) {
            if (x > bx)
                return false;
            fully_inside = false;
        } // rl
        Flt y = Abs(Dot(pos, T.matrix.y)) - T.size.y, by = BoxLengthAbs(ext.ext, plane_n_abs[DIR_UP]);
        if (y > -by) {
            if (y > by)
                return false;
            fully_inside = false;
        } // ud
        Flt z = Abs(Dot(pos, T.matrix.z)) - T.size.z, bz = BoxLengthAbs(ext.ext, plane_n_abs[DIR_FORWARD]);
        if (z > -bz) {
            if (z > bz)
                return false;
            fully_inside = false;
        } // fb
    }
    return true;
}
Bool FrustumClass::operator()(C Extent &ext, C Matrix3 &matrix, Bool &fully_inside) C {
    fully_inside = true;

    Vec dx = ext.ext.x * matrix.x,
        dy = ext.ext.y * matrix.y,
        dz = ext.ext.z * matrix.z,
#if 0
       pos=ext.pos*matrix-T.matrix.pos; // no need for 'VecD'
#else // faster than above
        pos = ext.pos;
    pos *= matrix;
    pos -= T.matrix.pos; // #VecMulMatrix
#endif

    if (persp) {
        Flt z = Dot(pos, T.matrix.z);
        // if( z<0 || z>range)
        {
            Flt bz = OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal);
            if (z < bz) {
                if (z < -bz)
                    return false;
                fully_inside = false;
            } // b
            if (z > range - bz) {
                if (z > range + bz)
                    return false;
                fully_inside = false;
            } // f
            MAX(z, 0);
        }

        Flt x = Dot(pos, T.matrix.x), bx = fov_tan.x * z + eye_dist_2;
        // if(Abs(x)>bx)
        {
            Flt bxr = OBoxLength(dx, dy, dz, plane[DIR_RIGHT].normal) * fov_cos_inv.x;
            if (x > bx - bxr) {
                if (x > bx + bxr)
                    return false;
                fully_inside = false;
            } // r
            Flt bxl = OBoxLength(dx, dy, dz, plane[DIR_LEFT].normal) * fov_cos_inv.x;
            if (x < -bx + bxl) {
                if (x < -bx - bxl)
                    return false;
                fully_inside = false;
            } // l
        }

        Flt y = Dot(pos, T.matrix.y), by = fov_tan.y * z;
        // if(Abs(y)>by)
        {
            Flt bxu = OBoxLength(dx, dy, dz, plane[DIR_UP].normal) * fov_cos_inv.y;
            if (y > by - bxu) {
                if (y > by + bxu)
                    return false;
                fully_inside = false;
            } // u
            Flt bxd = OBoxLength(dx, dy, dz, plane[DIR_DOWN].normal) * fov_cos_inv.y;
            if (y < -by + bxd) {
                if (y < -by - bxd)
                    return false;
                fully_inside = false;
            } // d
        }

        if (extra) {
            if (extraPlane()) {
                Flt e = Dist(pos + T.matrix.pos, extra_plane), be = OBoxLength(dx, dy, dz, extra_plane.normal);
                if (e > -be) {
                    if (e > be)
                        return false;
                    fully_inside = false;
                }
            }
            /* FIXME 'matrix' support
            if(extraBall())
            {
               Vec d=extra_ball.pos-ext.pos; d.abs(); // no need for 'VecD'

               if(Dist2(Max(0, d.x-ext.ext.x),
                        Max(0, d.y-ext.ext.y),
                        Max(0, d.z-ext.ext.z))>extra_ball_r2)return false; // check minimum distance, if overlapping

               if(Dist2(       d.x+ext.ext.x ,
                               d.y+ext.ext.y ,
                               d.z+ext.ext.z )>extra_ball_r2)fully_inside=false; // check maximum distance, if fully inside
            }*/
        }
    } else {
        Flt x = Abs(Dot(pos, T.matrix.x)) - T.size.x, bx = OBoxLength(dx, dy, dz, plane[DIR_RIGHT].normal);
        if (x > -bx) {
            if (x > bx)
                return false;
            fully_inside = false;
        } // rl
        Flt y = Abs(Dot(pos, T.matrix.y)) - T.size.y, by = OBoxLength(dx, dy, dz, plane[DIR_UP].normal);
        if (y > -by) {
            if (y > by)
                return false;
            fully_inside = false;
        } // ud
        Flt z = Abs(Dot(pos, T.matrix.z)) - T.size.z, bz = OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal);
        if (z > -bz) {
            if (z > bz)
                return false;
            fully_inside = false;
        } // fb
    }
    return true;
}
Bool FrustumClass::operator()(C Extent &ext, C Matrix &matrix, Bool &fully_inside) C {
    fully_inside = true;

    Vec dx = ext.ext.x * matrix.x,
        dy = ext.ext.y * matrix.y,
        dz = ext.ext.z * matrix.z,
#if 0
       pos=ext.pos*matrix-T.matrix.pos; // no need for 'VecD'
#else // faster than above
        pos = ext.pos;
    pos *= matrix;
    pos -= T.matrix.pos; // #VecMulMatrix
#endif

    if (persp) {
        Flt z = Dot(pos, T.matrix.z);
        // if( z<0 || z>range)
        {
            Flt bz = OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal);
            if (z < bz) {
                if (z < -bz)
                    return false;
                fully_inside = false;
            } // b
            if (z > range - bz) {
                if (z > range + bz)
                    return false;
                fully_inside = false;
            } // f
            MAX(z, 0);
        }

        Flt x = Dot(pos, T.matrix.x), bx = fov_tan.x * z + eye_dist_2;
        // if(Abs(x)>bx)
        {
            Flt bxr = OBoxLength(dx, dy, dz, plane[DIR_RIGHT].normal) * fov_cos_inv.x;
            if (x > bx - bxr) {
                if (x > bx + bxr)
                    return false;
                fully_inside = false;
            } // r
            Flt bxl = OBoxLength(dx, dy, dz, plane[DIR_LEFT].normal) * fov_cos_inv.x;
            if (x < -bx + bxl) {
                if (x < -bx - bxl)
                    return false;
                fully_inside = false;
            } // l
        }

        Flt y = Dot(pos, T.matrix.y), by = fov_tan.y * z;
        // if(Abs(y)>by)
        {
            Flt bxu = OBoxLength(dx, dy, dz, plane[DIR_UP].normal) * fov_cos_inv.y;
            if (y > by - bxu) {
                if (y > by + bxu)
                    return false;
                fully_inside = false;
            } // u
            Flt bxd = OBoxLength(dx, dy, dz, plane[DIR_DOWN].normal) * fov_cos_inv.y;
            if (y < -by + bxd) {
                if (y < -by - bxd)
                    return false;
                fully_inside = false;
            } // d
        }

        if (extra) {
            if (extraPlane()) {
                Flt e = Dist(pos + T.matrix.pos, extra_plane), be = OBoxLength(dx, dy, dz, extra_plane.normal);
                if (e > -be) {
                    if (e > be)
                        return false;
                    fully_inside = false;
                }
            }
            /* FIXME 'matrix' support
            if(extraBall())
            {
               Vec d=extra_ball.pos-ext.pos; d.abs(); // no need for 'VecD'

               if(Dist2(Max(0, d.x-ext.ext.x),
                        Max(0, d.y-ext.ext.y),
                        Max(0, d.z-ext.ext.z))>extra_ball_r2)return false; // check minimum distance, if overlapping

               if(Dist2(       d.x+ext.ext.x ,
                               d.y+ext.ext.y ,
                               d.z+ext.ext.z )>extra_ball_r2)fully_inside=false; // check maximum distance, if fully inside
            }*/
        }
    } else {
        Flt x = Abs(Dot(pos, T.matrix.x)) - T.size.x, bx = OBoxLength(dx, dy, dz, plane[DIR_RIGHT].normal);
        if (x > -bx) {
            if (x > bx)
                return false;
            fully_inside = false;
        } // rl
        Flt y = Abs(Dot(pos, T.matrix.y)) - T.size.y, by = OBoxLength(dx, dy, dz, plane[DIR_UP].normal);
        if (y > -by) {
            if (y > by)
                return false;
            fully_inside = false;
        } // ud
        Flt z = Abs(Dot(pos, T.matrix.z)) - T.size.z, bz = OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal);
        if (z > -bz) {
            if (z > bz)
                return false;
            fully_inside = false;
        } // fb
    }
    return true;
}
Bool FrustumClass::operator()(C Extent &ext, C MatrixM &matrix, Bool &fully_inside) C {
    fully_inside = true;

    Vec dx = ext.ext.x * matrix.x,
        dy = ext.ext.y * matrix.y,
        dz = ext.ext.z * matrix.z,
        pos = ext.pos * matrix - T.matrix.pos; // no need for 'VecD' if all computations done before setting to 'Vec'

    if (persp) {
        Flt z = Dot(pos, T.matrix.z);
        // if( z<0 || z>range)
        {
            Flt bz = OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal);
            if (z < bz) {
                if (z < -bz)
                    return false;
                fully_inside = false;
            } // b
            if (z > range - bz) {
                if (z > range + bz)
                    return false;
                fully_inside = false;
            } // f
            MAX(z, 0);
        }

        Flt x = Dot(pos, T.matrix.x), bx = fov_tan.x * z + eye_dist_2;
        // if(Abs(x)>bx)
        {
            Flt bxr = OBoxLength(dx, dy, dz, plane[DIR_RIGHT].normal) * fov_cos_inv.x;
            if (x > bx - bxr) {
                if (x > bx + bxr)
                    return false;
                fully_inside = false;
            } // r
            Flt bxl = OBoxLength(dx, dy, dz, plane[DIR_LEFT].normal) * fov_cos_inv.x;
            if (x < -bx + bxl) {
                if (x < -bx - bxl)
                    return false;
                fully_inside = false;
            } // l
        }

        Flt y = Dot(pos, T.matrix.y), by = fov_tan.y * z;
        // if(Abs(y)>by)
        {
            Flt bxu = OBoxLength(dx, dy, dz, plane[DIR_UP].normal) * fov_cos_inv.y;
            if (y > by - bxu) {
                if (y > by + bxu)
                    return false;
                fully_inside = false;
            } // u
            Flt bxd = OBoxLength(dx, dy, dz, plane[DIR_DOWN].normal) * fov_cos_inv.y;
            if (y < -by + bxd) {
                if (y < -by - bxd)
                    return false;
                fully_inside = false;
            } // d
        }

        if (extra) {
            if (extraPlane()) {
                Flt e = Dist(pos + T.matrix.pos, extra_plane), be = OBoxLength(dx, dy, dz, extra_plane.normal);
                if (e > -be) {
                    if (e > be)
                        return false;
                    fully_inside = false;
                }
            }
            /* FIXME 'matrix' support
            if(extraBall())
            {
               Vec d=extra_ball.pos-ext.pos; d.abs(); // no need for 'VecD'

               if(Dist2(Max(0, d.x-ext.ext.x),
                        Max(0, d.y-ext.ext.y),
                        Max(0, d.z-ext.ext.z))>extra_ball_r2)return false; // check minimum distance, if overlapping

               if(Dist2(       d.x+ext.ext.x ,
                               d.y+ext.ext.y ,
                               d.z+ext.ext.z )>extra_ball_r2)fully_inside=false; // check maximum distance, if fully inside
            }*/
        }
    } else {
        Flt x = Abs(Dot(pos, T.matrix.x)) - T.size.x, bx = OBoxLength(dx, dy, dz, plane[DIR_RIGHT].normal);
        if (x > -bx) {
            if (x > bx)
                return false;
            fully_inside = false;
        } // rl
        Flt y = Abs(Dot(pos, T.matrix.y)) - T.size.y, by = OBoxLength(dx, dy, dz, plane[DIR_UP].normal);
        if (y > -by) {
            if (y > by)
                return false;
            fully_inside = false;
        } // ud
        Flt z = Abs(Dot(pos, T.matrix.z)) - T.size.z, bz = OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal);
        if (z > -bz) {
            if (z > bz)
                return false;
            fully_inside = false;
        } // fb
    }
    return true;
}
/******************************************************************************/
Bool FrustumClass::operator()(C Box &box) C { return T(Extent(box)); }
Bool FrustumClass::operator()(C Box &box, C Matrix3 &matrix) C { return T(Extent(box), matrix); }
Bool FrustumClass::operator()(C Box &box, C Matrix &matrix) C { return T(Extent(box), matrix); }
Bool FrustumClass::operator()(C Box &box, C MatrixM &matrix) C { return T(Extent(box), matrix); }
Bool FrustumClass::operator()(C OBox &obox) C { return T(Extent(obox.box), obox.matrix); } // here we assume that 'obox.matrix' can be scaled
/******************************************************************************/
Bool FrustumClass::operator()(C Shape &shape) C {
    switch (shape.type) {
    case SHAPE_POINT:
        return T(shape.point);
    case SHAPE_BOX:
        return T(shape.box);
    case SHAPE_OBOX:
        return T(shape.obox);
    case SHAPE_BALL:
        return T(shape.ball);
    case SHAPE_CAPSULE:
        return T(shape.capsule);
    }
    return false;
}
Bool FrustumClass::operator()(C Shape *shape, Int shapes) C {
    REP(shapes)
    if (T(shape[i]))
        return true;
    return false;
}
/******************************************************************************/
Bool FrustumClass::operator()(C FrustumClass &frustum) C // assumes that one frustum is not entirely inside the other frustum (which means that there is contact between edge and faces)
{
    // comparing edges with faces is more precise than comparing points only
    REPD(f, 2) {
        C FrustumClass &a = (f ? T : frustum),
                       &b = (f ? frustum : T);
        REPD(e, a.edges) {
            VecD contact;
            EdgeD ed(a.point[a.edge[e].x], a.point[a.edge[e].y]);
            REPA(b.plane)
            if (Cuts(ed, b.plane[i], &contact))
                if (b(contact - b.plane[i].normal * EPSL))
                    return true; // use epsilon to make sure that we're under plane surface
        }
    }
    return false;
}
/******************************************************************************/
static INLINE void ProcessPos(C VecI2 &pos, C RectI &rect, Memt<VecI2> &row_min_max_x) {
    if (rect.includesY(pos.y)) {
        VecI2 &min_max_x = row_min_max_x[pos.y - rect.min.y];
        MIN(min_max_x.x, pos.x);
        MAX(min_max_x.y, pos.x);
    }
}
void FrustumClass::getIntersectingAreas(MemPtr<VecI2> area_pos, Flt area_size, Bool distance_check, Bool sort_by_distance, Bool extend, C RectI *clamp) C {
    area_pos.clear();

    Memt<VecD2> convex_points;
    CreateConvex2Dxz(convex_points, point, points);
    REPAO(convex_points) /= area_size;

    Bool mask_do;
    RectI mask, rect; // inclusive
    CircleM circle;
    if (clamp) {
        mask = *clamp;
        mask_do = true;
    } else
        mask_do = false;
    if (extraBall()) {
        circle.pos.x = extra_ball.pos.x / area_size;
        circle.pos.y = extra_ball.pos.z / area_size;
        circle.r = extra_ball.r / area_size;
        if (extend)
            circle.r += 0.5f;
        RectI r(Floor(circle.pos.x - circle.r), Floor(circle.pos.y - circle.r),
                Floor(circle.pos.x + circle.r), Floor(circle.pos.y + circle.r));
        if (mask_do)
            mask &= r;
        else {
            mask = r;
            mask_do = true;
        }
    }

    // clip convex, so we can calculate 'rect' precisely (after clipping, which may give different results instead of just "rect&mask") and so 'PixelWalker' doesn't have to walk too much
    // ---------
    //  \      |
    //   \     |
    //    \    |
    // +---\---| gets clipped into +---\---|
    // |    \  |                   |    \  |
    // |     \ |                   |     \ |
    // |      \|                   |      \|
    // +-------+                   +-------+
    if (mask_do) {
        Memt<VecD2> temp;
        PlaneD2 plane;
        plane.pos = mask.min;
        plane.normal.set(-1, 0);
        ClipPoly(convex_points, plane, temp); // left
        plane.normal.set(0, -1);
        ClipPoly(temp, plane, convex_points); // bottom
        plane.pos = mask.max + 1;
        plane.normal.set(1, 0);
        ClipPoly(convex_points, plane, temp); // right
        plane.normal.set(0, 1);
        ClipPoly(temp, plane, convex_points); // top
    }

    // set min_x..max_x, min_y..max_y visibility
    rect.setX(INT_MAX, INT_MIN); // set invalid "min>max"
    rect.setY(INT_MAX, INT_MIN); // set invalid "min>max"
    REPA(convex_points) {
        C VecD2 &p = convex_points[i];
        VecI2 pi;
        if (extend)
            pi.set(Floor(p.x - 0.5), Floor(p.y - 0.5));
        else
            pi.set(Floor(p.x), Floor(p.y));
        rect.include(pi);
    }
    if (extend)
        rect.max++;
    if (mask_do) {
        rect &= mask;
        if (!rect.valid())
            return;
    }

    // set min_x..max_x per row in 'row_min_max_x'
    Memt<VecI2> row_min_max_x;
    row_min_max_x.setNum(rect.h() + 1);         // +1 because it's inclusive
    REPAO(row_min_max_x).set(INT_MAX, INT_MIN); // on start set invalid range ("min>max")

    REPA(convex_points) // warning: this needs to work as well for "convex_points.elms()==1"
    {
        VecD2 start = convex_points[i], end = convex_points[(i + 1) % convex_points.elms()];
        if (extend) {
            // add corner point first as a 2x2 block (needs to process 2x2 because just 1 corner didn't cover all areas, the same was for Avg of 2 Perps)
            RectI corner;
            corner.min = Floor(start);
            VecI2 pos;
            if (start.x - corner.min.x < 0.5)
                corner.max.x = corner.min.x--;
            else
                corner.max.x = corner.min.x + 1; // if point is on the left   side (frac<0.5), then process from pos-1..pos, otherwise from pos..pos+1
            if (start.y - corner.min.y < 0.5)
                corner.max.y = corner.min.y--;
            else
                corner.max.y = corner.min.y + 1; // if point is on the bottom side (frac<0.5), then process from pos-1..pos, otherwise from pos..pos+1
            for (pos.y = corner.min.y; pos.y <= corner.max.y; pos.y++)
                for (pos.x = corner.min.x; pos.x <= corner.max.x; pos.x++)
                    ProcessPos(pos, rect, row_min_max_x);

            VecD2 perp = Perp(start - end);
            if (Dbl max = Abs(perp).max()) {
                perp *= 0.5 / max;
                start += perp;
                end += perp;
            } // use "Abs(perp).max()" instead of "perp.length()" because we need to extend orthogonally (because we're using extend for the purpose of detecting objects from neighborhood areas that extend over to other areas, and this extend is allowed orthogonally)
        }
        for (PixelWalker walker(start, end); walker.active(); walker.step())
            ProcessPos(walker.pos(), rect, row_min_max_x);
    }

    const Bool fast = true; // if use ~2x faster 'Dist2PointSquare' instead of 'Dist2(Vec2 point, RectI rect)'

    VecD2 distance_pos;
    Flt distance_range2;
    if (distance_check &= persp) // can do range tests only in perspective mode (orthogonal mode is used for shadows, and there we need full range, and also in that mode 'matrix.pos' is in the center, so it can't be used as 'distance_pos')
    {                            // convert to area space
        distance_pos = matrix.pos.xz() / area_size;
        if (fast)
            distance_pos -= 0.5; // since in fast mode we're testing against a square of radius 0.5, instead of setting each square as "pos+0.5", we offset the 'distance_pos' by the negative (this was tested and works OK - the same results as when 'fast'=false)
        distance_range2 = range / area_size;
        if (extend)
            distance_range2 += 0.5f;
        distance_range2 *= distance_range2;
    }
    if (extraBall()) {
        if (fast)
            circle.pos -= 0.5; // since in fast mode we're testing against a square of radius 0.5, instead of setting each square as "pos+0.5", we offset the 'circle.pos' by the negative (this was tested and works OK - the same results as when 'fast'=false)
        SQR(circle.r);
    }

    // set areas for drawing
    if (sort_by_distance) // in look order (from camera/foreground to background)
    {
        Vec2 look_dir = matrix.z.xz();
        Flt max = Abs(look_dir).max();
        VecI2 dir = (max ? Round(look_dir / max) : VecI2(0, 1)), // (-1, -1) .. (1, 1)
            perp = Perp(dir);                                    // parallel to direction
        if ((dir.x == 1 && dir.y == 1) || (dir.x == 1 && dir.y == 0) || (dir.x == -1 && dir.y == -1) || (dir.x == 0 && dir.y == -1))
            perp.chs();

        for (VecI2 edge((dir.x < 0) ? rect.max.x : rect.min.x, (dir.y < 0) ? rect.max.y : rect.min.y);;) {
            for (VecI2 pos = edge;;) {
                VecI2 &min_max_x = row_min_max_x[pos.y - rect.min.y];
                if (pos.x >= min_max_x.x && pos.x <= min_max_x.y)
                    if (!distance_check || (fast ? Dist2PointSquare(distance_pos, pos, 0.5) : Dist2(distance_pos, RectI(pos, pos + 1))) <= distance_range2)
                        if (!extraBall() || (fast ? Dist2PointSquare(circle.pos, pos, 0.5) : Dist2(circle.pos, RectI(pos, pos + 1))) <= circle.r)
                            area_pos.add(pos); // add to array

                pos += perp;
                if (!rect.includes(pos))
                    break; // go along the parallel until you can't
            }
            if (dir.x && rect.includesX(edge.x + dir.x))
                edge.x += dir.x;
            else // first travel on the x-edge until you can't
                if (dir.y && rect.includesY(edge.y + dir.y))
                    edge.y += dir.y;
                else
                    break; // then  travel on the y-edge until you can't, after that get out of the loop
        }
    } else {
        VecI2 pos;
        for (pos.y = rect.min.y; pos.y <= rect.max.y; pos.y++) {
            VecI2 min_max_x = row_min_max_x[pos.y - rect.min.y];
            MAX(min_max_x.x, rect.min.x);
            MIN(min_max_x.y, rect.max.x);
            for (pos.x = min_max_x.x; pos.x <= min_max_x.y; pos.x++)
                if (!distance_check || (fast ? Dist2PointSquare(distance_pos, pos, 0.5) : Dist2(distance_pos, RectI(pos, pos + 1))) <= distance_range2)
                    if (!extraBall() || (fast ? Dist2PointSquare(circle.pos, pos, 0.5) : Dist2(circle.pos, RectI(pos, pos + 1))) <= circle.r)
                        area_pos.add(pos); // add to array
        }
    }
}
/******************************************************************************/
void FrustumClass::getIntersectingSphereAreas(MemPtr<SphereArea> area_pos, C SphereConvertEx &sc, Bool distance_check, Bool sort_by_distance, Bool extend, Flt min_radius) C {
    // WARNING: ignores 'extraBall'
    area_pos.clear();
    distance_check &= persp; // can do range tests only in perspective mode (orthogonal mode is used for shadows, and there we need full range, and also in that mode 'matrix.pos' is in the center)

    Memt<VecD2> convex_points;
    Memt<VecI2> row_min_max_x;
    Memt<VecD2> temp;
    Memt<SphereAreaDist> area_pos_dist;
    SpherePixelWalker walker(sc);
    SphereArea ap;
    RectI rect;
    rect.setX(0, sc.res - 1);
    Bool distance_check_y = false;
    Dbl half = 1.0 / sc.res;     // range is from -1..1, range=2, half=1
    Dbl min_height = min_radius; // this is treated orthogonally, require points to be at least above this value, if they're not, then detect intersections on edges at this height
    /*               *
                *         *
             *               *
           *                   *
           \                  /
            \                /
             \              /
 min_height - \------------/
            |  \          /
            |   \        /
            |    \      /
            |     \    /
            |      \  /
            -       \/
    */

    for (ap.side = DIR_ENUM(0);;) {
        {
            Ball oriented_ball;
            Flt r2;
            VecI2 ball_cell;
            if (distance_check) {
                PosToTerrainPos(ap.side, oriented_ball.pos, matrix.pos);
                oriented_ball.r = range;
                if (!ClipZ(oriented_ball, min_radius))
                    goto next;
            }

            VecD oriented_point[ELMS(point)]; // point converted to 'ap.side' orientation where XY=plane position, Z=height
            Bool oriented_point_ok[ELMS(point)];
            VecD2 projected_point[ELMS(point) + ELMS(edge)]; // point projected on plane XY, with Z=1 (think of Box/Cube where each side is treated as plane/spherical grid), can be created from each point and edge
            Int projected_points = 0;
            PosToTerrainPos(ap.side, oriented_point, point, points);
            REP(points) {
                C auto &src = oriented_point[i];
                if (oriented_point_ok[i] = (src.z >= min_height))
                    projected_point[projected_points++] = src.xy / src.z; // project onto XY plane with Z=1
            }
            if (projected_points < points) // if not all points got projected, then check edges
            {
                REP(edges) {
                    VecI2 edge = T.edge[i];
                    if (oriented_point_ok[edge.x] != oriented_point_ok[edge.y]) // if one point is OK and other NOT (one above min_height and one under)
                    {
                        C auto &a = oriented_point[edge.x], &b = oriented_point[edge.y];           // Edge a->b
                        Dbl delta = b.z - a.z, frac = (min_height - a.z) / delta;                  // calculate Lerp frac to intersect Edge at Z=min_height
                        projected_point[projected_points++] = Lerp(a.xy, b.xy, frac) / min_height; // project point from Z=min_height to Z=1 (this Lerp generates point at intersection Z=min_height)
                    }
                }
                if (!projected_points)
                    goto next; // if none then skip
            }
            CreateConvex2D(convex_points, projected_point, projected_points);

            // clip convex, so we can calculate 'rect' precisely (after clipping, which may give different results instead of just "rect&mask") and so 'SpherePixelWalker' doesn't have to walk too much
            // ---------
            //  \      |
            //   \     |
            //    \    |
            // +---\---| gets clipped into +---\---|
            // |    \  |                   |    \  |
            // |     \ |                   |     \ |
            // |      \|                   |      \|
            // +-------+                   +-------+
            {
                Rect rect(-1, 1);
                if (distance_check) // clamp to ball rect, this is needed to reduce processing (skip checking areas out of rect range) and because in the loop later we check only area corners against ball (but not area sides)
                {
                    Flt len2, sin2, cos;
                    Vec zd, d, test;

                    // oriented_ball/=oriented_ball.pos.z; // project to plane XY with Z=1
                    if (extend)
                        oriented_ball.r += half * oriented_ball.pos.z; // must be proportional to height
                    r2 = Sqr(oriented_ball.r);
                    ball_cell = sc.posToCellI(oriented_ball.pos.xy / oriented_ball.pos.z);

                    zd.set(oriented_ball.pos.x, 0, oriented_ball.pos.z);
                    len2 = zd.length2();
                    if (r2 < len2) {
                        sin2 = r2 / len2;
                        cos = CosSin2(sin2);
                        d = CrossUp(zd);
                        d.setLength(cos * oriented_ball.r);
                        zd *= -sin2;
                        zd += oriented_ball.pos;
                        test = zd - d;
                        if (test.z > 0)
                            MAX(rect.min.x, test.x / test.z);
                        test = zd + d;
                        if (test.z > 0)
                            MIN(rect.max.x, test.x / test.z);
                    }

                    zd.set(0, oriented_ball.pos.y, oriented_ball.pos.z);
                    len2 = zd.length2();
                    if (r2 < len2) {
                        sin2 = r2 / len2;
                        cos = CosSin2(sin2);
                        d = CrossRight(zd);
                        d.setLength(cos * oriented_ball.r);
                        zd *= -sin2;
                        zd += oriented_ball.pos;
                        test = zd + d;
                        if (test.z > 0)
                            MAX(rect.min.y, test.y / test.z);
                        test = zd - d;
                        if (test.z > 0)
                            MIN(rect.max.y, test.y / test.z);
                    }
                }
                PlaneD2 plane;
                plane.pos = rect.min;
                plane.normal.set(-1, 0);
                ClipPoly(convex_points, plane, temp); // left
                plane.normal.set(0, -1);
                ClipPoly(temp, plane, convex_points); // bottom
                plane.pos = rect.max;
                plane.normal.set(1, 0);
                ClipPoly(convex_points, plane, temp); // right
                plane.normal.set(0, 1);
                ClipPoly(temp, plane, convex_points); // top
            }

            // set min_y..max_y visibility
            rect.setY(INT_MAX, INT_MIN); // set invalid "min>max"
            REPA(convex_points) {
                VecD2 &p = convex_points[i];
                Int y = sc.posToCellI(extend ? p.y - half : p.y); // don't use 'posToCellIMid', instead do 'clampY' below just one time
                rect.includeY(y);
            }
            if (extend)
                rect.max.y++;
            rect.clampY(0, sc.res - 1);
            if (!rect.validY())
                goto next;

            // set min_x..max_x per row in 'row_min_max_x'
            row_min_max_x.setNumDiscard(rect.h() + 1);  // +1 because it's inclusive
            REPAO(row_min_max_x).set(INT_MAX, INT_MIN); // on start set invalid range ("min>max")

            REPA(convex_points) // warning: this needs to work as well for "convex_points.elms()==1"
            {
                VecD2 start = convex_points[i], end = convex_points[(i + 1) % convex_points.elms()];
                if (extend) {
                    // add corner point first as a 2x2 block (needs to process 2x2 because just 1 corner didn't cover all areas, the same was for Avg of 2 Perps)
                    auto cell = sc.posToCell(start);
                    RectI corner;
                    corner.min = Floor(cell);
                    VecI2 pos;
                    if (cell.x - corner.min.x < 0.5f)
                        corner.max.x = corner.min.x--;
                    else
                        corner.max.x = corner.min.x + 1; // if point is on the left   side (frac<0.5f), then process from pos-1..pos, otherwise from pos..pos+1, here use 0.5 instead of 'half', because this is in cell space and not real space
                    if (cell.y - corner.min.y < 0.5f)
                        corner.max.y = corner.min.y--;
                    else
                        corner.max.y = corner.min.y + 1; // if point is on the bottom side (frac<0.5f), then process from pos-1..pos, otherwise from pos..pos+1, here use 0.5 instead of 'half', because this is in cell space and not real space
                    for (pos.y = corner.min.y; pos.y <= corner.max.y; pos.y++)
                        for (pos.x = corner.min.x; pos.x <= corner.max.x; pos.x++)
                            ProcessPos(pos, rect, row_min_max_x);

                    VecD2 perp = Perp(start - end);
                    if (Dbl max = Abs(perp).max()) {
                        perp *= half / max;
                        start += perp;
                        end += perp;
                    } // use "Abs(perp).max()" instead of "perp.length()" because we need to extend orthogonally (because we're using extend for the purpose of detecting objects from neighborhood areas that extend over to other areas, and this extend is allowed orthogonally)
                }
#if DEBUG && 0
                D.line(PINK, start, end);
#endif
                for (walker.start(start, end); walker.active(); walker.step())
                    ProcessPos(walker.posi(), rect, row_min_max_x);
            }

            // set areas for drawing
#if 0 // can't do this because this is sorting just for one cube side, but we need to sort for all sides/areas
         if(sort_by_distance) // in look order (from camera/foreground to background)
         {
            // set min_x..max_x visibility
            rect.setX(INT_MAX, INT_MIN); // set invalid "min>max"
            REPA(convex_points)
            {
               VecD2 &p=convex_points[i];
               Int    x=sc.posToCellI(extend ? p.x-half : p.x); // don't use 'posToCellIMid', instead do 'clampX' below just one time
               rect.includeX(x);
            }
            if(extend)rect.max.x++;
            rect.clampX(0, sc.res-1);
            if(!rect.validX())goto next;

            Vec2  look_dir; PosToTerrainPos(ap.side, look_dir, matrix.z);
            Flt   max =Abs(look_dir).max();
            VecI2 dir =(max ? Round(look_dir/max) : VecI2(0, 1)), // (-1, -1) .. (1, 1)
                  perp=Perp(dir);                                 // parallel to direction
            if((dir.x== 1 && dir.y== 1)
            || (dir.x== 1 && dir.y== 0)
            || (dir.x==-1 && dir.y==-1)
            || (dir.x== 0 && dir.y==-1))perp.chs();

            for(VecI2 edge((dir.x<0) ? rect.max.x : rect.min.x, (dir.y<0) ? rect.max.y : rect.min.y); ; )
            {
               for(ap=edge; ; )
               {
                  VecI2 &min_max_x=row_min_max_x[ap.y-rect.min.y];
                  if(ap.x>=min_max_x.x && ap.x<=min_max_x.y)
                     distance_check
                        area_pos.add(ap); // add to array

                  ap+=perp; if(!rect.includes(ap))break; // go along the parallel until you can't
               }
               if(dir.x && rect.includesX(edge.x+dir.x))edge.x+=dir.x;else        // first travel on the x-edge until you can't
               if(dir.y && rect.includesY(edge.y+dir.y))edge.y+=dir.y;else break; // then  travel on the y-edge until you can't, after that get out of the loop
            }
         }else
#endif
            for (ap.y = rect.min.y; ap.y <= rect.max.y; ap.y++) {
                Flt cell_pos_y;
                if (distance_check) {
                    distance_check_y = (ap.y != ball_cell.y);                // if ball is on down or up side
                    cell_pos_y = sc._cellToPos(ap.y + (ap.y < ball_cell.y)); // (ap.y<ball_cell.y) ? _cellToPos(ap.y+1) : _cellToPos(ap.y)
                }

                VecI2 min_max_x = row_min_max_x[ap.y - rect.min.y];
                MAX(min_max_x.x, rect.min.x);
                MIN(min_max_x.y, rect.max.x);
                for (ap.x = min_max_x.x; ap.x <= min_max_x.y; ap.x++) {
                    if (distance_check_y        // if ball is on down or up    side and 'distance_check'
                        && ap.x != ball_cell.x) // if ball is on left or right side
                    {
                        // left =(ap.x>ball_cell.x); if ball is on the left  side of area
                        // right=(ap.x<ball_cell.x); if ball is on the right side of area
                        // down =(ap.y>ball_cell.y); if ball is on the down  side of area
                        // up   =(ap.y<ball_cell.y); if ball is on the up    side of area
                        Vec dir(sc._cellToPos(ap.x + (ap.x < ball_cell.x)), // (ap.x<ball_cell.x) ? _cellToPos(ap.x+1) : _cellToPos(ap.x)
                                cell_pos_y,                                 // (ap.y<ball_cell.y) ? _cellToPos(ap.y+1) : _cellToPos(ap.y)
                                1);
                        dir.normalize();
                        if (Dist2PointLine(oriented_ball.pos, dir) >= r2)
                            continue;
                    }
                    if (sort_by_distance) // in look order (from camera/foreground to background)
                    {
                        SphereAreaDist &apd = area_pos_dist.New();
                        SCAST(SphereArea, apd) = ap;
                        Vec dir = sc._sphereTerrainPixelCenterToDir(apd.side, apd.x, apd.y);
                        apd.dist = Dot(matrix.z, dir) * RSqrt0(dir.length2()); // '_sphereTerrainPixelCenterToDir' should be normalized, but here we can just use fast approximation
                    } else {
                        area_pos.add(ap); // add to array
                    }
                }
            }
        }
    next:
        if (ap.side == DIR_NUM - 1)
            break;
        ap.side = DIR_ENUM(ap.side + 1);
    }

    if (sort_by_distance) {
        area_pos_dist.sort(Compare);
        area_pos.setNum(area_pos_dist.elms());
        REPAO(area_pos) = area_pos_dist[i];
    }
}
/******************************************************************************/
void FrustumClass::draw(C Color &col) C {
    VI.color(col);
    REP(edges)
    VI.line(point[edge[i].x], point[edge[i].y]);
    VI.end();

    // REPA(plane)DrawArrow(RED, plane[i].p, plane[i].p+plane[i].normal*3);
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
