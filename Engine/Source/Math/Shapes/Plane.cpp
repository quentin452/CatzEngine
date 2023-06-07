/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
static void DrawPlane(C Color &color, Flt cell_size, Int resolution, C VecD &center, C Matrix3 &matrix, Bool ball)
{
   Flt  size=cell_size*resolution/2;
   VecD lf  =center+size*( matrix.z-matrix.x),
        lb  =center+size*(-matrix.z-matrix.x),
        rb  =center+size*( matrix.x-matrix.z);
   VI.color(color);
   if(ball)
   {          
      BallM ball(D.viewRange(), CamMatrix.pos/ObjMatrix);
      REP(resolution+1)
      {
         Flt  step=i*cell_size;
         Vec  dx  =step*matrix.x,
              dz  =step*matrix.z;
         VecD a, b;
         if(CutsLineBall(lb+dx, matrix.z, ball, &a, &b)==2)VI.line(a, b);
         if(CutsLineBall(lb+dz, matrix.x, ball, &a, &b)==2)VI.line(a, b);
      }
   }else
   {
      REP(resolution+1)
      {
         Flt step=i*cell_size;
         Vec dx  =step*matrix.x,
             dz  =step*matrix.z;
         VI.line(lf+dx, lb+dx);
         VI.line(lb+dz, rb+dz);
      }
   }
   VI.end();
}
void Plane::drawLocal(C Color &color, Flt size, Bool fill, Int resolution)C
{
   Matrix3 matrix; matrix.setUp(normal);
   if(fill)
   {
      Quad(pos+size*(matrix.z-matrix.x), pos+size*(matrix.x+matrix.z), pos+size*(matrix.x-matrix.z), pos+size*(-matrix.x-matrix.z)).draw(color, true);
   }else
   {
      if(resolution<0)resolution=16;else if(resolution<2)resolution=2;
      DrawPlane(color, (size*2)/resolution, resolution, pos, matrix, false);
   }
}
void Plane::drawInfiniteBySize(C Color &color, Flt cell_size)C
{
   Matrix3 matrix; matrix.setUp(normal);
   Int     resolution=Ceil2(Ceil((D.viewRange()*2)/cell_size)); // must be multiple of 2
   Dbl    _cell_size =cell_size;
   VecD    cam_delta =CamMatrix.pos/ObjMatrix-pos,
           aligned   =pos+matrix.x*AlignRound(Dot(cam_delta, matrix.x), _cell_size)
                         +matrix.z*AlignRound(Dot(cam_delta, matrix.z), _cell_size);

   DrawPlane(color, cell_size, resolution, aligned, matrix, true);
}
void Plane::drawInfiniteByResolution(C Color &color, Int resolution)C
{
   if(resolution<0)resolution=32;else if(resolution<2)resolution=2;

   Matrix3 matrix; matrix.setUp(normal);
   Dbl     cell_size=(D.viewRange()*2)/resolution;
   VecD    cam_delta=CamMatrix.pos/ObjMatrix-pos,
           aligned  =pos+matrix.x*AlignRound(Dot(cam_delta, matrix.x), cell_size)
                        +matrix.z*AlignRound(Dot(cam_delta, matrix.z), cell_size);

   DrawPlane(color, cell_size, resolution, aligned, matrix, true);
}
/******************************************************************************/
Flt DistPointPlane(C Vec2  &point, C Vec2  &plane_pos, C Vec2  &plane_normal) {return plane_normal.x*(point.x-plane_pos.x) + plane_normal.y*(point.y-plane_pos.y);}
Dbl DistPointPlane(C VecD2 &point, C VecD2 &plane_pos, C VecD2 &plane_normal) {return plane_normal.x*(point.x-plane_pos.x) + plane_normal.y*(point.y-plane_pos.y);}
Flt DistPointPlane(C Vec   &point, C Vec   &plane_pos, C Vec   &plane_normal) {return plane_normal.x*(point.x-plane_pos.x) + plane_normal.y*(point.y-plane_pos.y) + plane_normal.z*(point.z-plane_pos.z);}
Dbl DistPointPlane(C Vec   &point, C VecD  &plane_pos, C Vec   &plane_normal) {return plane_normal.x*(point.x-plane_pos.x) + plane_normal.y*(point.y-plane_pos.y) + plane_normal.z*(point.z-plane_pos.z);}
Dbl DistPointPlane(C VecD  &point, C Vec   &plane_pos, C Vec   &plane_normal) {return plane_normal.x*(point.x-plane_pos.x) + plane_normal.y*(point.y-plane_pos.y) + plane_normal.z*(point.z-plane_pos.z);}
Dbl DistPointPlane(C VecD  &point, C VecD  &plane_pos, C Vec   &plane_normal) {return plane_normal.x*(point.x-plane_pos.x) + plane_normal.y*(point.y-plane_pos.y) + plane_normal.z*(point.z-plane_pos.z);}
Dbl DistPointPlane(C VecD  &point, C VecD  &plane_pos, C VecD  &plane_normal) {return plane_normal.x*(point.x-plane_pos.x) + plane_normal.y*(point.y-plane_pos.y) + plane_normal.z*(point.z-plane_pos.z);}

// skip testing "Dot!=0" before division for performance reasons
Flt DistPointPlaneRay(C Vec2  &point,                     C Vec2  &plane_normal, C Vec2  &ray) {return Dot           (point,            plane_normal)/Dot(ray, plane_normal);}
Dbl DistPointPlaneRay(C VecD2 &point,                     C VecD2 &plane_normal, C VecD2 &ray) {return Dot           (point,            plane_normal)/Dot(ray, plane_normal);}
Flt DistPointPlaneRay(C Vec   &point,                     C Vec   &plane_normal, C Vec   &ray) {return Dot           (point,            plane_normal)/Dot(ray, plane_normal);}
Dbl DistPointPlaneRay(C VecD  &point,                     C Vec   &plane_normal, C Vec   &ray) {return Dot           (point,            plane_normal)/Dot(ray, plane_normal);}
Dbl DistPointPlaneRay(C VecD  &point,                     C VecD  &plane_normal, C VecD  &ray) {return Dot           (point,            plane_normal)/Dot(ray, plane_normal);}
Flt DistPointPlaneRay(C Vec2  &point, C Vec2  &plane_pos, C Vec2  &plane_normal, C Vec2  &ray) {return DistPointPlane(point, plane_pos, plane_normal)/Dot(ray, plane_normal);}
Dbl DistPointPlaneRay(C VecD2 &point, C VecD2 &plane_pos, C VecD2 &plane_normal, C VecD2 &ray) {return DistPointPlane(point, plane_pos, plane_normal)/Dot(ray, plane_normal);}
Flt DistPointPlaneRay(C Vec   &point, C Vec   &plane_pos, C Vec   &plane_normal, C Vec   &ray) {return DistPointPlane(point, plane_pos, plane_normal)/Dot(ray, plane_normal);}
Dbl DistPointPlaneRay(C VecD  &point, C VecD  &plane_pos, C Vec   &plane_normal, C Vec   &ray) {return DistPointPlane(point, plane_pos, plane_normal)/Dot(ray, plane_normal);}
Dbl DistPointPlaneRay(C VecD  &point, C VecD  &plane_pos, C VecD  &plane_normal, C VecD  &ray) {return DistPointPlane(point, plane_pos, plane_normal)/Dot(ray, plane_normal);}

Flt DistPointPlaneY(C Vec  &point,                    C Vec  &plane_normal) {return Dot           (point,            plane_normal)/plane_normal.y;}
Dbl DistPointPlaneY(C VecD &point,                    C Vec  &plane_normal) {return Dot           (point,            plane_normal)/plane_normal.y;}
Dbl DistPointPlaneY(C VecD &point,                    C VecD &plane_normal) {return Dot           (point,            plane_normal)/plane_normal.y;}
Flt DistPointPlaneY(C Vec  &point, C Vec  &plane_pos, C Vec  &plane_normal) {return DistPointPlane(point, plane_pos, plane_normal)/plane_normal.y;}
Dbl DistPointPlaneY(C VecD &point, C VecD &plane_pos, C Vec  &plane_normal) {return DistPointPlane(point, plane_pos, plane_normal)/plane_normal.y;}
Dbl DistPointPlaneY(C VecD &point, C VecD &plane_pos, C VecD &plane_normal) {return DistPointPlane(point, plane_pos, plane_normal)/plane_normal.y;}

Vec2  PointOnPlane   (C Vec2  &point,                     C Vec2  &plane_normal              ) {return point-plane_normal*DistPointPlane   (point,            plane_normal     );}
VecD2 PointOnPlane   (C VecD2 &point,                     C VecD2 &plane_normal              ) {return point-plane_normal*DistPointPlane   (point,            plane_normal     );}
Vec   PointOnPlane   (C Vec   &point,                     C Vec   &plane_normal              ) {return point-plane_normal*DistPointPlane   (point,            plane_normal     );}
VecD  PointOnPlane   (C VecD  &point,                     C Vec   &plane_normal              ) {return point-plane_normal*DistPointPlane   (point,            plane_normal     );}
VecD  PointOnPlane   (C VecD  &point,                     C VecD  &plane_normal              ) {return point-plane_normal*DistPointPlane   (point,            plane_normal     );}
Vec2  PointOnPlane   (C Vec2  &point, C Vec2  &plane_pos, C Vec2  &plane_normal              ) {return point-plane_normal*DistPointPlane   (point, plane_pos, plane_normal     );}
VecD2 PointOnPlane   (C VecD2 &point, C VecD2 &plane_pos, C VecD2 &plane_normal              ) {return point-plane_normal*DistPointPlane   (point, plane_pos, plane_normal     );}
Vec   PointOnPlane   (C Vec   &point, C Vec   &plane_pos, C Vec   &plane_normal              ) {return point-plane_normal*DistPointPlane   (point, plane_pos, plane_normal     );}
VecD  PointOnPlane   (C Vec   &point, C VecD  &plane_pos, C Vec   &plane_normal              ) {return point-plane_normal*DistPointPlane   (point, plane_pos, plane_normal     );}
VecD  PointOnPlane   (C VecD  &point, C Vec   &plane_pos, C Vec   &plane_normal              ) {return point-plane_normal*DistPointPlane   (point, plane_pos, plane_normal     );}
VecD  PointOnPlane   (C VecD  &point, C VecD  &plane_pos, C Vec   &plane_normal              ) {return point-plane_normal*DistPointPlane   (point, plane_pos, plane_normal     );}
VecD  PointOnPlane   (C VecD  &point, C VecD  &plane_pos, C VecD  &plane_normal              ) {return point-plane_normal*DistPointPlane   (point, plane_pos, plane_normal     );}
Vec2  PointOnPlaneRay(C Vec2  &point,                     C Vec2  &plane_normal, C Vec2  &ray) {return point-ray         *DistPointPlaneRay(point,            plane_normal, ray);}
VecD2 PointOnPlaneRay(C VecD2 &point,                     C VecD2 &plane_normal, C VecD2 &ray) {return point-ray         *DistPointPlaneRay(point,            plane_normal, ray);}
Vec   PointOnPlaneRay(C Vec   &point,                     C Vec   &plane_normal, C Vec   &ray) {return point-ray         *DistPointPlaneRay(point,            plane_normal, ray);}
VecD  PointOnPlaneRay(C VecD  &point,                     C Vec   &plane_normal, C Vec   &ray) {return point-ray         *DistPointPlaneRay(point,            plane_normal, ray);}
VecD  PointOnPlaneRay(C VecD  &point,                     C VecD  &plane_normal, C VecD  &ray) {return point-ray         *DistPointPlaneRay(point,            plane_normal, ray);}
Vec2  PointOnPlaneRay(C Vec2  &point, C Vec2  &plane_pos, C Vec2  &plane_normal, C Vec2  &ray) {return point-ray         *DistPointPlaneRay(point, plane_pos, plane_normal, ray);}
VecD2 PointOnPlaneRay(C VecD2 &point, C VecD2 &plane_pos, C VecD2 &plane_normal, C VecD2 &ray) {return point-ray         *DistPointPlaneRay(point, plane_pos, plane_normal, ray);}
Vec   PointOnPlaneRay(C Vec   &point, C Vec   &plane_pos, C Vec   &plane_normal, C Vec   &ray) {return point-ray         *DistPointPlaneRay(point, plane_pos, plane_normal, ray);}
VecD  PointOnPlaneRay(C VecD  &point, C VecD  &plane_pos, C Vec   &plane_normal, C Vec   &ray) {return point-ray         *DistPointPlaneRay(point, plane_pos, plane_normal, ray);}
VecD  PointOnPlaneRay(C VecD  &point, C VecD  &plane_pos, C VecD  &plane_normal, C VecD  &ray) {return point-ray         *DistPointPlaneRay(point, plane_pos, plane_normal, ray);}
Vec2   ZeroOnPlaneRay(                C Vec2  &plane_pos, C Vec2  &plane_normal, C Vec2  &ray) {return       ray         *DistPointPlaneRay(       plane_pos, plane_normal, ray);}
Vec    ZeroOnPlaneRay(                C Vec   &plane_pos, C Vec   &plane_normal, C Vec   &ray) {return       ray         *DistPointPlaneRay(       plane_pos, plane_normal, ray);}

Vec  PointOnPlaneY(C Vec  &point,                    C Vec  &plane_normal) {return Vec (point.x, point.y-DistPointPlaneY(point,            plane_normal), point.z);}
VecD PointOnPlaneY(C VecD &point,                    C Vec  &plane_normal) {return VecD(point.x, point.y-DistPointPlaneY(point,            plane_normal), point.z);}
VecD PointOnPlaneY(C VecD &point,                    C VecD &plane_normal) {return VecD(point.x, point.y-DistPointPlaneY(point,            plane_normal), point.z);}
Vec  PointOnPlaneY(C Vec  &point, C Vec  &plane_pos, C Vec  &plane_normal) {return Vec (point.x, point.y-DistPointPlaneY(point, plane_pos, plane_normal), point.z);}
VecD PointOnPlaneY(C VecD &point, C VecD &plane_pos, C Vec  &plane_normal) {return VecD(point.x, point.y-DistPointPlaneY(point, plane_pos, plane_normal), point.z);}
VecD PointOnPlaneY(C VecD &point, C VecD &plane_pos, C VecD &plane_normal) {return VecD(point.x, point.y-DistPointPlaneY(point, plane_pos, plane_normal), point.z);}

Flt   PointOnPlane(  Flt    a,   Flt    b, Flt dist_a, Flt dist_b) {return Lerp(a, b, PointOnPlaneStep(dist_a, dist_b));}
Dbl   PointOnPlane(  Dbl    a,   Dbl    b, Dbl dist_a, Dbl dist_b) {return Lerp(a, b, PointOnPlaneStep(dist_a, dist_b));}
Vec2  PointOnPlane(C Vec2  &a, C Vec2  &b, Flt dist_a, Flt dist_b) {return Lerp(a, b, PointOnPlaneStep(dist_a, dist_b));}
VecD2 PointOnPlane(C VecD2 &a, C VecD2 &b, Dbl dist_a, Dbl dist_b) {return Lerp(a, b, PointOnPlaneStep(dist_a, dist_b));}
Vec   PointOnPlane(C Vec   &a, C Vec   &b, Flt dist_a, Flt dist_b) {return Lerp(a, b, PointOnPlaneStep(dist_a, dist_b));}
VecD  PointOnPlane(C VecD  &a, C VecD  &b, Dbl dist_a, Dbl dist_b) {return Lerp(a, b, PointOnPlaneStep(dist_a, dist_b));}
/******************************************************************************/
Bool SweepPointPlane(C Vec2 &point, C Vec2 &move, C Plane2 &plane, Flt *hit_frac, Vec2 *hit_normal, Vec2 *hit_pos, Bool two_sided)
{
   Flt way =Dot (move , plane.normal); if(two_sided ? (way==0) : (way>=0))return false; // we ignore cases when the point is already on the plane to allow movement along the plane surface, in 'two_sided' mode we skip if we move along the plane surface "way==0", in one sided mode we skip if we move along the plane surface "way==0" or in the same direction as plane normal "way>0"
   Flt dist=Dist(point, plane       );
   if((way<0)                // if we're going towards the plane (against plane normal)
   ? (dist<0 || dist+way>0)  // distance is already below the plane OR even after movement we still haven't reached the plane
   : (dist>0 || dist+way<0)) // distance is already above the plane OR even after movement we still haven't reached the plane
      return false;
 //if(hit_frac || hit_pos) skip testing this as it's faster this way
   {
      Flt frac=-dist/way;
      if(hit_frac  ) *hit_frac  =      frac;
      if(hit_pos   ) *hit_pos   =point+frac*move;
   }  if(hit_normal){*hit_normal=plane.normal; if(way>0)hit_normal->chs();}
   return true;
}
Bool SweepPointPlane(C VecD2 &point, C VecD2 &move, C PlaneD2 &plane, Dbl *hit_frac, VecD2 *hit_normal, VecD2 *hit_pos, Bool two_sided)
{
   Dbl way =Dot (move , plane.normal); if(two_sided ? (way==0) : (way>=0))return false; // we ignore cases when the point is already on the plane to allow movement along the plane surface, in 'two_sided' mode we skip if we move along the plane surface "way==0", in one sided mode we skip if we move along the plane surface "way==0" or in the same direction as plane normal "way>0"
   Dbl dist=Dist(point, plane       );
   if((way<0)                // if we're going towards the plane (against plane normal)
   ? (dist<0 || dist+way>0)  // distance is already below the plane OR even after movement we still haven't reached the plane
   : (dist>0 || dist+way<0)) // distance is already above the plane OR even after movement we still haven't reached the plane
      return false;
 //if(hit_frac || hit_pos) skip testing this as it's faster this way
   {
      Dbl frac=-dist/way;
      if(hit_frac  ) *hit_frac  =      frac;
      if(hit_pos   ) *hit_pos   =point+frac*move;
   }  if(hit_normal){*hit_normal=plane.normal; if(way>0)hit_normal->chs();}
   return true;
}
Bool SweepPointPlane(C Vec &point, C Vec &move, C Plane &plane, Flt *hit_frac, Vec *hit_normal, Vec *hit_pos, Bool two_sided)
{
   Flt way =Dot (move , plane.normal); if(two_sided ? (way==0) : (way>=0))return false; // we ignore cases when the point is already on the plane to allow movement along the plane surface, in 'two_sided' mode we skip if we move along the plane surface "way==0", in one sided mode we skip if we move along the plane surface "way==0" or in the same direction as plane normal "way>0"
   Flt dist=Dist(point, plane       );
   if((way<0)                // if we're going towards the plane (against plane normal)
   ? (dist<0 || dist+way>0)  // distance is already below the plane OR even after movement we still haven't reached the plane
   : (dist>0 || dist+way<0)) // distance is already above the plane OR even after movement we still haven't reached the plane
      return false;
 //if(hit_frac || hit_pos) skip testing this as it's faster this way
   {
      Flt frac=-dist/way;
      if(hit_frac  ) *hit_frac  =      frac;
      if(hit_pos   ) *hit_pos   =point+frac*move;
   }  if(hit_normal){*hit_normal=plane.normal; if(way>0)hit_normal->chs();}
   return true;
}
Bool SweepPointPlane(C VecD &point, C VecD &move, C PlaneD &plane, Dbl *hit_frac, VecD *hit_normal, VecD *hit_pos, Bool two_sided)
{
   Dbl way =Dot (move , plane.normal); if(two_sided ? (way==0) : (way>=0))return false; // we ignore cases when the point is already on the plane to allow movement along the plane surface, in 'two_sided' mode we skip if we move along the plane surface "way==0", in one sided mode we skip if we move along the plane surface "way==0" or in the same direction as plane normal "way>0"
   Dbl dist=Dist(point, plane       );
   if((way<0)                // if we're going towards the plane (against plane normal)
   ? (dist<0 || dist+way>0)  // distance is already below the plane OR even after movement we still haven't reached the plane
   : (dist>0 || dist+way<0)) // distance is already above the plane OR even after movement we still haven't reached the plane
      return false;
 //if(hit_frac || hit_pos) skip testing this as it's faster this way
   {
      Dbl frac=-dist/way;
      if(hit_frac  ) *hit_frac  =      frac;
      if(hit_pos   ) *hit_pos   =point+frac*move;
   }  if(hit_normal){*hit_normal=plane.normal; if(way>0)hit_normal->chs();}
   return true;
}
/******************************************************************************/
Bool CutsLinePlane(C Vec &line_pos, C Vec &line_dir, C Plane &plane, Flt *hit_frac, Vec *hit_normal, Vec *hit_pos, Bool two_sided)
{
   Flt way=Dot(line_dir, plane.normal); if(two_sided ? (way==0) : (way>=0))return false; // we ignore cases when the point is already on the plane to allow movement along the plane surface, in 'two_sided' mode we skip if we move along the plane surface "way==0", in one sided mode we skip if we move along the plane surface "way==0" or in the same direction as plane normal "way>0"
   if(hit_frac || hit_pos)
   {
      Flt dist=Dist(line_pos, plane), frac=-dist/way;
      if(hit_frac  ) *hit_frac  =         frac;
      if(hit_pos   ) *hit_pos   =line_pos+frac*line_dir;
   }  if(hit_normal){*hit_normal=plane.normal; if(way>0)hit_normal->chs();}
   return true;
}
Bool CutsLinePlane(C VecD &line_pos, C VecD &line_dir, C PlaneD &plane, Dbl *hit_frac, VecD *hit_normal, VecD *hit_pos, Bool two_sided)
{
   Dbl way=Dot(line_dir, plane.normal); if(two_sided ? (way==0) : (way>=0))return false; // we ignore cases when the point is already on the plane to allow movement along the plane surface, in 'two_sided' mode we skip if we move along the plane surface "way==0", in one sided mode we skip if we move along the plane surface "way==0" or in the same direction as plane normal "way>0"
   if(hit_frac || hit_pos)
   {
      Dbl dist=Dist(line_pos, plane), frac=-dist/way;
      if(hit_frac  ) *hit_frac  =         frac;
      if(hit_pos   ) *hit_pos   =line_pos+frac*line_dir;
   }  if(hit_normal){*hit_normal=plane.normal; if(way>0)hit_normal->chs();}
   return true;
}
/******************************************************************************/
Bool Intersect(C Plane &a, C Plane &b, Vec &line_pos, Vec &line_dir)
{
   line_dir=Cross(a.normal, b.normal); if(!line_dir.normalize())return false;
   Vec  ray=Cross(a.normal, line_dir); // ray is normalized already because both parameters are perp to each other
   line_pos=PointOnPlaneRay(a.pos, b.pos, b.normal, ray);
   return true;
}
/******************************************************************************/
void SlideMovement(Vec2 &move, C Vec2 *normal, Int normals)
{
   Flt dist=0; Int find=-1; REP(normals) // find biggest distance (furthest from zero) that's <0 (intersecting)
   {
      Flt d=Dot(move, normal[i]);
      if( d<dist){dist=d; find=i;} // if intersecting (and distance bigger than previous)
   }
   if(find>=0)
   {
    C Vec2 &furthest_nrm=normal[find];
      move-=dist*furthest_nrm;
      REP(normals)if(i!=find) // check all normals but skip the one we processed already
      {
       C Vec2 &nrm=normal[i];
         Flt dot=Dot(move, nrm);
         if( dot                   <-EPS      // if still intersecting ( fully)
         || (dot                   <0         // if still intersecting (barely)
          && Dot(furthest_nrm, nrm)<EPS_COS)) // and not the same normal
            {move.zero(); break;} // can't move
      }
   }
}
/******************************************************************************/
void SlideMovement(Vec &move, C Vec *normal, Int normals)
{
   Vec  slide;
   Bool slide_valid=false, hit0=false;
   REPD(i0, normals)
   {
    C Vec &nrm0=normal[i0];
      Flt d=Dot(move, nrm0); if(d<0) // if intersecting
      {
              hit0  =true ; // have any intersection
         Bool hit1  =false;
         Vec  slide0=move-d*nrm0;
         REPD(i1, normals)if(i0!=i1) // check all normals but skip the one we processed already
         {
          C Vec &nrm1=normal[i1];
            Flt dot=Dot(slide0, nrm1);
            if( dot            <-EPS      // if still intersecting ( fully)
            || (dot            <0         // if still intersecting (barely)
             && Dot(nrm0, nrm1)<EPS_COS)) // and not the same normal
            {
                   hit1   =true;
               Vec slide1 =CrossN(nrm0, nrm1  );
                   slide1*=Dot   (move, slide1);
               REPD(i2, normals)if(i0!=i2 && i1!=i2) // check all normals but skip the one we processed already
               {
                C Vec &nrm2=normal[i2];
                  Flt dot=Dot(slide1, nrm2);
                  if( dot            <-EPS      // if still intersecting ( fully)
                  || (dot            <0         // if still intersecting (barely)
                   && Dot(nrm0, nrm2)<EPS_COS   // and not the same normal
                   && Dot(nrm1, nrm2)<EPS_COS)) // and not the same normal
                     goto hit2; // can't move this way
               }
               if(!slide_valid || slide1.length2()>slide.length2()){slide_valid=true; slide=slide1;}
            hit2:;
            }
         }
         if(!hit1)
         {
            if(!slide_valid || slide0.length2()>slide.length2()){slide_valid=true; slide=slide0;}
         }
      }
   }
   if(hit0)
   {
      if(slide_valid)move=slide;else move.zero();
   }
}
/******************************************************************************
This doesn't always find the movement
void SlideMovement1(Vec &move, C Vec *normal, Int normals)
{
   Flt dist=0; Int i0=-1; REP(normals) // find biggest distance (furthest from zero) that's <0 (intersecting)
   {
      Flt d=Dot(move, normal[i]);
      if( d<dist){dist=d; i0=i;} // if intersecting (and distance bigger than previous)
   }
   if(i0>=0)
   {
    C Vec  &nrm0=normal[i0];
      Vec slide0=move-dist*nrm0;
      dist=0; Int i1=-1; REP(normals)if(i!=i0) // check all normals but skip the one we processed already
      {
       C Vec &nrm=normal[i];
         Flt d=Dot(slide0, nrm);
         if( d<dist){dist=d; i1=i;} // if intersecting (and distance bigger than previous)
      }
      if(i1>=0)
      {
       C Vec &nrm1=normal[i1];
         if(dist           <-EPS     // big distance
         || Dot(nrm0, nrm1)<EPS_COS) // not the same normal
         {
            Vec slide1 =CrossN(nrm0, nrm1  );
                slide1*=Dot   (move, slide1);
            REPD(i2, normals)if(i0!=i2 && i1!=i2) // check all normals but skip the one we processed already
            {
             C Vec &nrm2=normal[i2];
               Flt dot=Dot(slide1, nrm2);
               if( dot            <-EPS      // if still intersecting ( fully)
               || (dot            <0         // if still intersecting (barely)
                && Dot(nrm0, nrm2)<EPS_COS   // and not the same normal
                && Dot(nrm1, nrm2)<EPS_COS)) // and not the same normal
                  {move.zero(); return;} // can't move this way
            }
            move=slide1; return;
         }
      }
      move=slide0;
   }
}
static void Test()
{
   Random.seed.set(1,2,3,4);
   REP(10000000)
   {
      Vec normal[6]; Int normals=Random(6);
      REP(normals)normal[i]=Random(Ball(1), false);
      Vec dir=Random(Ball(1), false);
      Vec d0=dir; SlideMovement (d0, normal, normals);
      Vec d1=dir; SlideMovement1(d1, normal, normals);
      REP(normals)
      {
         Flt dot;
         dot=Dot(d0, normal[i]); if(dot<-EPS)
            int z=0;
         dot=Dot(d1, normal[i]); if(dot<-EPS)
            int z=0;
      }
      Flt D0=Dot(dir, !d0);
      Flt D1=Dot(dir, !d1);
      if(!Equal(d0.length(), D0, 0.001f))
         int z=0;
      if(!Equal(d1.length(), D1, 0.001f))
         int z=0;
      if(!Equal(d0, d1))
      {
         d1=dir; SlideMovement1(d1, normal, normals);
      }
   }
}
/******************************************************************************/
VecD2 SlideMovement(C CircleM &object, C VecD2 &move, Int vtxs, C Vec2 *vtx_pos, Int edges, C VecI2 *edge_ind, C Vec2 *edge_nrm, Int circles, C CircleM *circle, Bool slide)
{
#define DBL_PREC 1 // double precision needed because there are problems in Flt (object moves along the edge, then is getting blocked by that edge vertex with Dot(nrm, hit_nrm_prev)=0.999758601 which is too big)
#if     DBL_PREC
   typedef Dbl   Real;
   typedef VecD2 VecR2;
   #define EPS_NRM_DOT EPS_COS
#else
   typedef Flt  Real;
   typedef Vec2 VecR2;
   #define EPS_NRM_DOT 0.999995f // current value was needed in tests, don't increase !! using EPS_COS 0.9999995f was insufficient, this needs to be lower because it depends on a lot of calculations, so error is higher
#endif

   if(Real move_len=move.length2())
   {
      move_len=SqrtFast(move_len);
      Int   step=0;
      Real  object_r    =object.r,
            object_r_eps=object_r-EPS; // this will ignore vertexes that are barely touching object along the movement line, this is useful for cases when first slid along edge, but then detected collision with the same edges vertex (due to numerical precision issues)
      VecR2 cur_pos=object.pos;
      VecR2 move_n=move/move_len;
      VecR2 hit_nrm_prev;
   again:
      Real  hit_len=move_len; // find only smaller than move length
      VecR2 hit_nrm;
      // edges
      REPD(e, edges)
      {
       C VecI2 &edge=edge_ind[e];
       C VecR2 &edge_p0=vtx_pos[edge.x],
               &edge_p1=vtx_pos[edge.y];
      #if DBL_PREC
         VecR2  edge_n=(edge_nrm ? (VecR2)edge_nrm[e] : PerpN(edge_p0-edge_p1));
      #else
         VecR2  edge_nrm_temp;
       C VecR2 &edge_n=(edge_nrm ? edge_nrm[e] : edge_nrm_temp=PerpN(edge_p0-edge_p1));
      #endif
         Real   move_dot=Dot(move_n, edge_n); if(move_dot<0) // if moving towards edge
         {
            Real dist_nrm=DistPointPlane(cur_pos, edge_p0, edge_n); if(dist_nrm>=0) // if outside the edge
            {
               if(step && Dot(edge_n, hit_nrm_prev)>=EPS_NRM_DOT)goto ignore_edge; // if same normal as from previous step, then ignore this edge

                  dist_nrm-=object_r;
               if(dist_nrm<=0) // object intersecting edge
               {
                  VecR2 edge_dir=Perp(edge_n); // p0->p1 (p1-p0)
                  if(DistPointPlane(cur_pos, edge_p0, edge_dir)>=0
                  && DistPointPlane(cur_pos, edge_p1, edge_dir)<=0)
                  {
                     cur_pos-=edge_n*dist_nrm; // move object along edge normal to avoid intersection
                     hit_len=0;
                     hit_nrm=edge_n;
                     goto hit_found;
                  }
                  // Don't check edge points here, instead check them later in points section
               }else
               {
                  Real dist_move=-dist_nrm/move_dot; if(dist_move<hit_len) // if less than previous distance
                  {
                     VecR2 closest=cur_pos-edge_n*object_r; // closest point on object to the edge
                           closest+=move_n*dist_move;
                     VecR2 edge_dir=Perp(edge_n); // p0->p1 (p1-p0)
                     if(DistPointPlane(closest, edge_p0, edge_dir)>=0
                     && DistPointPlane(closest, edge_p1, edge_dir)<=0)
                     {
                        hit_len=dist_move;
                        hit_nrm=edge_n;
                     }
                  }
               }
            }
         }
      ignore_edge:;
      }
      // points and circles
      {
         VecR2 move_r=Perp(move_n);
         VecR2 object_rel(Dot(cur_pos, move_r), Dot(cur_pos, move_n)); // object position in space relative to movement
         REPD(v, vtxs)
         {
          C VecR2 &pos=vtx_pos[v];
            VecR2  pos_rel;
            pos_rel.x=Dot(pos, move_r)-object_rel.x; // check X first because this will eliminate more vertexes
            if(Abs(pos_rel.x)<object_r_eps) // consider only vertexes along the movement line
            {
               pos_rel.y=Dot(pos, move_n); // check Y
               if(pos_rel.y>object_rel.y) // test only > and not >= because if it's =0 then it means cur_pos and vtx_pos are the same and movement would always be stuck no matter what direction
               {
                  Real dist_move=pos_rel.y-object_rel.y; // !! Warning: this is not yet complete !!
                  if(  dist_move<hit_len+object_r) // if can potentially have smaller distance
                  {
                        dist_move-=CosSin(pos_rel.x/object_r)*object_r; // finish calculation
                     if(dist_move<hit_len) // if less than previous distance
                     {
                        if(dist_move<=0) // object intersecting point
                        {
                           VecR2 delta=cur_pos-pos; Real delta_len2=delta.length2(); if(!delta_len2)goto ignore_vtx; // if length is zero then cur pos and vtx pos are the same so just ignore it, this check is also needed for div by 0
                         //if(delta_len2<Sqr(object_r)) shouldn't be needed at this stage
                           {
                              Real  delta_len=SqrtFast(delta_len2);
                              VecR2 nrm=delta/delta_len;
                              if(step && Dot(nrm, hit_nrm_prev)>=EPS_NRM_DOT)goto ignore_vtx; // if same normal as from previous step, then ignore this vertex

                              hit_len=0;
                              hit_nrm=nrm;
                              cur_pos+=hit_nrm*(object_r-delta_len); // move object away from point
                              goto hit_found;
                           }
                        }

                        VecR2 nrm=(cur_pos+move_n*dist_move)-pos; nrm.normalize();
                        if(step && Dot(nrm, hit_nrm_prev)>=EPS_NRM_DOT)goto ignore_vtx; // if same normal as from previous step, then ignore this vertex

                        hit_len=dist_move;
                        hit_nrm=nrm;
                     }
                  }
               }
            }
         ignore_vtx:;
         }
         REPD(c, circles)
         {
          C auto  &circ=circle[c];
            Real   r   =circ.r+object_r;
          C VecR2 &pos =circ.pos;
            VecR2  pos_rel;
            pos_rel.x=Dot(pos, move_r)-object_rel.x; // check X first because this will eliminate more circles
            if(Abs(pos_rel.x)<r) // consider only circles along the movement line
            {
               pos_rel.y=Dot(pos, move_n); // check Y
               if(pos_rel.y>object_rel.y) // test only > and not >= because if it's =0 then it means cur_pos and circle_pos are the same and movement would always be stuck no matter what direction
               {
                  Real dist_move=pos_rel.y-object_rel.y; // !! Warning: this is not yet complete !!
                  if(  dist_move<hit_len+r) // if can potentially have smaller distance
                  {
                        dist_move-=CosSin(pos_rel.x/r)*r; // finish calculation
                     if(dist_move<hit_len) // if less than previous distance
                     {
                        if(dist_move<=0) // object intersecting circle
                        {
                           VecR2 delta=cur_pos-pos; Real delta_len2=delta.length2(); if(!delta_len2)goto ignore_circle; // if length is zero then cur pos and circle pos are the same so just ignore it, this check is also needed for div by 0
                         //if(delta_len2<Sqr(r)) shouldn't be needed at this stage
                           {
                              Real  delta_len=SqrtFast(delta_len2);
                              VecR2 nrm=delta/delta_len;
                              if(step && Dot(nrm, hit_nrm_prev)>=EPS_NRM_DOT)goto ignore_circle; // if same normal as from previous step, then ignore this circle

                              hit_len=0;
                              hit_nrm=nrm;
                              cur_pos+=hit_nrm*(r-delta_len); // move object away from circle
                              goto hit_found;
                           }
                        }

                        VecR2 nrm=(cur_pos+move_n*dist_move)-pos; nrm.normalize();
                        if(step && Dot(nrm, hit_nrm_prev)>=EPS_NRM_DOT)goto ignore_circle; // if same normal as from previous step, then ignore this circle

                        hit_len=dist_move;
                        hit_nrm=nrm;
                     }
                  }
               }
            }
         ignore_circle:;
         }
      }
      if(hit_len<move_len) // collision
      {
      hit_found:
         cur_pos+=move_n*hit_len;
         if(step<Int(slide))
         {
            move_len-= hit_len; // moved this already
            move_n  *=move_len; // remaining move
            move_n  -=Dot(move_n, hit_nrm)*hit_nrm;
            if(move_len=move_n.normalize())
            {
               step++;
               hit_nrm_prev=hit_nrm;
               goto again;
            }
         }
         return cur_pos;
      }
      return cur_pos+(step ? move_n*hit_len : move); // no collision = move all the way
   }
   return object.pos;
}
/******************************************************************************/
}
/******************************************************************************/
