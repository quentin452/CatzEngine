/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Capsule& Capsule::operator*=(Flt f) {pos*=f; r*=f; h*=f; return T;}
Capsule& Capsule::operator/=(Flt f) {pos/=f; r/=f; h/=f; return T;}

CapsuleM& CapsuleM::operator*=(Flt f) {pos*=f; r*=f; h*=f; return T;}
CapsuleM& CapsuleM::operator/=(Flt f) {pos/=f; r/=f; h/=f; return T;}

Capsule& Capsule::operator*=(C Vec &v)
{
   Matrix3 m; m.setUp(up)*=v;
   h*=    m.y.length();
   r*=Avg(m.x.length(), m.z.length());
   pos*=v;
   return T;
}
Capsule& Capsule::operator/=(C Vec &v)
{
   Matrix3 m; m.setUp(up)/=v;
   h*=    m.y.length();
   r*=Avg(m.x.length(), m.z.length());
   pos/=v;
   return T;
}

CapsuleM& CapsuleM::operator*=(C Vec &v)
{
   Matrix3 m; m.setUp(up)*=v;
   h*=    m.y.length();
   r*=Avg(m.x.length(), m.z.length());
   pos*=v;
   return T;
}
CapsuleM& CapsuleM::operator/=(C Vec &v)
{
   Matrix3 m; m.setUp(up)/=v;
   h*=    m.y.length();
   r*=Avg(m.x.length(), m.z.length());
   pos/=v;
   return T;
}

/******************************************************************************/
Capsule& Capsule::operator*=(C Matrix3 &m)
{
   pos*=m;
   up *=m;
   h  *=up.normalize();
   r  *=m .avgScale ();
   return T;
}
Capsule& Capsule::operator/=(C Matrix3 &m)
{
   pos/=m;
   up /=m;
   h  *=up.normalize(); // 'h' should indeed be multiplied here because 'up' already got transformed in correct way
   r  /=m .avgScale ();
   return T;
}
Capsule& Capsule::operator*=(C Matrix &m)
{
   pos*=m;
   up *=m .orn();
   h  *=up.normalize();
   r  *=m .avgScale ();
   return T;
}
Capsule& Capsule::operator/=(C Matrix &m)
{
   pos/=m;
   up /=m .orn();
   h  *=up.normalize(); // 'h' should indeed be multiplied here because 'up' already got transformed in correct way
   r  /=m .avgScale ();
   return T;
}
Capsule& Capsule::operator*=(C MatrixM &m)
{
   pos*=m;
   up *=m .orn();
   h  *=up.normalize();
   r  *=m .avgScale ();
   return T;
}
Capsule& Capsule::operator/=(C MatrixM &m)
{
   pos/=m;
   up /=m .orn();
   h  *=up.normalize(); // 'h' should indeed be multiplied here because 'up' already got transformed in correct way
   r  /=m .avgScale ();
   return T;
}
/******************************************************************************/
CapsuleM& CapsuleM::operator*=(C Matrix3 &m)
{
   pos*=m;
   up *=m;
   h  *=up.normalize();
   r  *=m .avgScale ();
   return T;
}
CapsuleM& CapsuleM::operator/=(C Matrix3 &m)
{
   pos/=m;
   up /=m;
   h  *=up.normalize(); // 'h' should indeed be multiplied here because 'up' already got transformed in correct way
   r  /=m .avgScale ();
   return T;
}
CapsuleM& CapsuleM::operator*=(C Matrix &m)
{
   pos*=m;
   up *=m .orn();
   h  *=up.normalize();
   r  *=m .avgScale ();
   return T;
}
CapsuleM& CapsuleM::operator/=(C Matrix &m)
{
   pos/=m;
   up /=m .orn();
   h  *=up.normalize(); // 'h' should indeed be multiplied here because 'up' already got transformed in correct way
   r  /=m .avgScale ();
   return T;
}
CapsuleM& CapsuleM::operator*=(C MatrixM &m)
{
   pos*=m;
   up *=m .orn();
   h  *=up.normalize();
   r  *=m .avgScale ();
   return T;
}
CapsuleM& CapsuleM::operator/=(C MatrixM &m)
{
   pos/=m;
   up /=m .orn();
   h  *=up.normalize(); // 'h' should indeed be multiplied here because 'up' already got transformed in correct way
   r  /=m .avgScale ();
   return T;
}
/******************************************************************************/
Capsule& Capsule::set(Flt r, C Vec &from, C Vec &to)
{
   T.r  =r;
   T.pos=Avg(from, to);
   T.up =to-from;
   T.h  =T.up.normalize()+r*2;
   return T;
}
Capsule& Capsule::setEdge(Flt r, C Vec &from, C Vec &to)
{
   T.r  =r;
   T.pos=Avg(from, to);
   T.up =to-from;
   T.h  =T.up.normalize();
   return T;
}
/******************************************************************************/
Vec Capsule::nearestNrm(C Vec &normal)C
{
   return isBall() ? pos - normal*ballR()
                   : pos - normal*r - up*( Sign(Dot(up, normal))*innerHeightHalf() );
}
VecD CapsuleM::nearestNrm(C Vec &normal)C
{
   return isBall() ? pos - normal*ballR()
                   : pos - normal*r - up*( Sign(Dot(up, normal))*innerHeightHalf() );
}
Vec Capsule::nearestPos(C Vec &pos, Bool inside)C
{
   Vec nearest; if(isBall())nearest=T.pos;else
   {
      Flt ihh=innerHeightHalf(), y=Mid(DistPointPlane(pos, T.pos, up), -ihh, ihh);
      nearest=T.pos+up*y;
   }
   Vec dir=pos-nearest;
   Flt len2=dir.length2();
   if(inside ? len2<=Sqr(r) : !len2)return pos; // inside or div by zero
   return nearest+dir*(r/SqrtFast(len2));
}
VecD CapsuleM::nearestPos(C VecD &pos, Bool inside)C
{
   VecD nearest; if(isBall())nearest=T.pos;else
   {
      Dbl ihh=innerHeightHalf(), y=Mid(DistPointPlane(pos, T.pos, up), -ihh, ihh);
      nearest=T.pos+up*y;
   }
   VecD dir=pos-nearest;
   Dbl  len2=dir.length2();
   if(inside ? len2<=Sqr(r) : !len2)return pos; // inside or div by zero
   return nearest+dir*(r/SqrtFast(len2));
}
/******************************************************************************/
void Capsule::draw(C Color &color, Bool fill, Int resolution)C
{
   if(isBall())Ball(T).draw(color, fill, resolution);else
   {
      if(resolution<0)resolution=24;else if(resolution<3)resolution=3;
      Tube t(r, h-r*2, pos, up); t.draw(color, fill, resolution); t.up*=t.h*0.5f;
      Ball  (r, pos+t.up).drawAngle(color, 0,  PI_2, up, fill, VecI2(resolution, resolution/3));
      Ball  (r, pos-t.up).drawAngle(color, 0, -PI_2, up, fill, VecI2(resolution, resolution/3));
   }
}
/******************************************************************************/
Flt Dist(C Vec &point, C Capsule &capsule)
{
                            if(capsule.isBall())return Max(0, Dist         (point, capsule.pos                   )-capsule.ballR());
   Vec up=capsule.up*capsule.innerHeightHalf(); return Max(0, DistPointEdge(point, capsule.pos-up, capsule.pos+up)-capsule.r      ); // 'DistPointEdge' is safe in case edge is zero length
}
Dbl Dist(C VecD &point, C CapsuleM &capsule)
{
                            if(capsule.isBall())return Max(0, Dist         (point, capsule.pos                   )-capsule.ballR());
   Vec up=capsule.up*capsule.innerHeightHalf(); return Max(0, DistPointEdge(point, capsule.pos-up, capsule.pos+up)-capsule.r      ); // 'DistPointEdge' is safe in case edge is zero length
}
Flt Dist(C Edge &edge, C Capsule &capsule)
{
   return Max(0, capsule.isBall() ? Dist(capsule.pos       , edge)-capsule.ballR()
                                  : Dist(capsule.ballEdge(), edge)-capsule.r);
}
Dbl Dist(C EdgeD &edge, C CapsuleM &capsule)
{
   return Max(0, capsule.isBall() ? Dist(capsule.pos       , edge)-capsule.ballR()
                                  : Dist(capsule.ballEdge(), edge)-capsule.r);
}
Flt Dist(C TriN &tri, C Capsule &capsule)
{
   return Max(0, capsule.isBall() ? Dist(capsule.pos       , tri)-capsule.ballR()
                                  : Dist(capsule.ballEdge(), tri)-capsule.r);
}
Flt Dist(C Box &box, C Capsule &capsule)
{
   return Max(0, capsule.isBall() ? Dist(capsule.pos       , box)-capsule.ballR()
                                  : Dist(capsule.ballEdge(), box)-capsule.r);
}
Flt Dist(C OBox &obox, C Capsule &capsule)
{
   if(capsule.isBall())
   {
      Ball ball;
      ball.r=capsule.ballR();
      ball.pos.fromDivNormalized(capsule.pos, obox.matrix);
      return Dist(obox.box, ball);
   }
   Capsule temp=capsule; // temp = capsule in box local space
   temp.pos.divNormalized(obox.matrix      );
   temp.up .divNormalized(obox.matrix.orn());
   return Dist(obox.box, temp);
}
Flt Dist(C Ball &ball, C Capsule &capsule)
{
   return Max(0, capsule.isBall() ? Dist(ball.pos, capsule.pos       )-capsule.ballR()-ball.r
                                  : Dist(ball.pos, capsule.ballEdge())-capsule.r      -ball.r);
}
Flt Dist(C Capsule &a, C Capsule &b)
{
   return Max(0, (a.isBall() ? b.isBall() ? Dist(a.pos       , b.pos       )
                                          : Dist(a.pos       , b.ballEdge())
                             : b.isBall() ? Dist(b.pos       , a.ballEdge())
                                          : Dist(a.ballEdge(), b.ballEdge())
                 )-a.r-b.r); // !! WARNING: for simplicity this assumes that ballR==r !!
}
Flt DistFull(C Capsule &a, C Capsule &b)
{
   return (a.isBall() ? b.isBall() ? Dist(a.pos       , b.pos       )
                                   : Dist(a.pos       , b.ballEdge())
                      : b.isBall() ? Dist(b.pos       , a.ballEdge())
                                   : Dist(a.ballEdge(), b.ballEdge())
          )-a.r-b.r; // !! WARNING: for simplicity this assumes that ballR==r !!
}
Flt DistCapsulePlane(C Capsule &capsule, C Vec &plane, C Vec &normal)
{
   return DistPointPlane(capsule.nearestNrm(normal), plane, normal);
}
Dbl DistCapsulePlane(C Capsule &capsule, C VecD &plane, C Vec &normal)
{
   return DistPointPlane(capsule.nearestNrm(normal), plane, normal);
}
Dbl DistCapsulePlane(C CapsuleM &capsule, C VecD &plane, C Vec &normal)
{
   return DistPointPlane(capsule.nearestNrm(normal), plane, normal);
}
/******************************************************************************/
Bool Cuts(C Vec &point, C Capsule &capsule)
{
   if(capsule.isBall())return Dist2(point, capsule.pos)<=Sqr(capsule.ballR());
   Vec up=capsule.up*capsule.innerHeightHalf();
   return Dist2PointEdge(point, capsule.pos-up, capsule.pos+up)<=Sqr(capsule.r);
}
Bool Cuts(C VecD &point, C Capsule &capsule)
{
   if(capsule.isBall())return Dist2(point, capsule.pos)<=Sqr(capsule.ballR());
   Vec up=capsule.up*capsule.innerHeightHalf();
   return Dist2PointEdge(point, capsule.pos-up, capsule.pos+up)<=Sqr(capsule.r);
}
Bool Cuts(C VecD &point, C CapsuleM &capsule)
{
   if(capsule.isBall())return Dist2(point, capsule.pos)<=Sqr(capsule.ballR());
   Vec up=capsule.up*capsule.innerHeightHalf();
   return Dist2PointEdge(point, capsule.pos-up, capsule.pos+up)<=Sqr(capsule.r);
}
Bool Cuts(C Edge &edge, C Capsule &capsule)
{
   if(capsule.isBall())return Dist2(capsule.pos, edge)<=Sqr(capsule.ballR());
   return Dist2(capsule.ballEdge(), edge)<=Sqr(capsule.r);
}
Bool Cuts(C Tri &tri, C Capsule &capsule)
{
   if(capsule.isBall())return Dist2(capsule.pos, tri)<=Sqr(capsule.ballR());
   return Dist2(capsule.ballEdge(), tri)<=Sqr(capsule.r);
}
Bool Cuts(C TriN &tri, C Capsule &capsule)
{
   if(capsule.isBall())return Dist2(capsule.pos, tri)<=Sqr(capsule.ballR());
   return Dist2(capsule.ballEdge(), tri)<=Sqr(capsule.r);
}
Bool Cuts(C Box &box, C Capsule &capsule)
{
   if(capsule.isBall())return Dist2(capsule.pos, box)<=Sqr(capsule.ballR());
   return Dist2(capsule.ballEdge(), box)<=Sqr(capsule.r);
}
Bool Cuts(C OBox &obox, C Capsule &capsule)
{
   if(capsule.isBall())
   {
      Ball ball;
      ball.r=capsule.ballR();
      ball.pos.fromDivNormalized(capsule.pos, obox.matrix);
      return Dist2(ball.pos, obox.box)<=Sqr(ball.r);
   }
   Capsule temp=capsule;
   temp.pos.divNormalized(obox.matrix      );
   temp.up .divNormalized(obox.matrix.orn());
   return Dist2(temp.ballEdge(), obox.box)<=Sqr(temp.r);
}
Bool Cuts(C Ball &ball, C Capsule &capsule)
{
   if(capsule.isBall())return Dist2(ball.pos, capsule.pos)<=Sqr(ball.r+capsule.ballR());
   return Dist2(ball.pos, capsule.ballEdge())<=Sqr(capsule.r+ball.r);
}
Bool Cuts(C BallM &ball, C Capsule &capsule)
{
   if(capsule.isBall())return Dist2(ball.pos, capsule.pos)<=Sqr(ball.r+capsule.ballR());
   return Dist2(ball.pos, capsule.ballEdge())<=Sqr(capsule.r+ball.r);
}
Bool Cuts(C BallM &ball, C CapsuleM &capsule)
{
   if(capsule.isBall())return Dist2(ball.pos, capsule.pos)<=Sqr(ball.r+capsule.ballR());
   return Dist2(ball.pos, capsule.ballEdge())<=Sqr(capsule.r+ball.r);
}
Bool Cuts(C Capsule &a, C Capsule &b)
{
   return (a.isBall() ? b.isBall() ? Dist2(a.pos       , b.pos       )
                                   : Dist2(a.pos       , b.ballEdge())
                      : b.isBall() ? Dist2(b.pos       , a.ballEdge())
                                   : Dist2(a.ballEdge(), b.ballEdge())
          )<=Sqr(a.r+b.r); // !! WARNING: for simplicity this assumes that ballR==r !!
}
/******************************************************************************/
Bool SweepPointCapsule(C Vec &point, C Vec &move, C Capsule &capsule, Flt *hit_frac, Vec *hit_normal)
{
   return capsule.isBall() ? SweepBallPoint(Ball(capsule.ballR(), point), move, capsule.pos       , hit_frac, hit_normal)
                           : SweepBallEdge (Ball(capsule.r      , point), move, capsule.ballEdge(), hit_frac, hit_normal);
}
Bool SweepBallCapsule(C Ball &ball, C Vec &move, C Capsule &capsule, Flt *hit_frac, Vec *hit_normal)
{
   return capsule.isBall() ? SweepBallPoint(Ball(ball.r+capsule.ballR(), ball.pos), move, capsule.pos       , hit_frac, hit_normal)
                           : SweepBallEdge (Ball(ball.r+capsule.r      , ball.pos), move, capsule.ballEdge(), hit_frac, hit_normal);
}
/******************************************************************************/
Bool SweepCapsuleEdge(C Capsule &capsule, C Vec &move, C Edge &edge, Flt *hit_frac, Vec *hit_normal)
{
   if(capsule.isBall())return SweepBallEdge(Ball(capsule), move, edge, hit_frac, hit_normal);

   Bool   check;
   Flt    ihh=capsule.innerHeightHalf();
   Matrix matrix; matrix.    setPosDir(capsule.pos, capsule.up);
   Edge2  edge2  (matrix.      convert(edge.p[0]  , true), matrix.convert(edge.p[1], true));
   Vec2   move2  =matrix.orn().convert(move       , true);
   Vec2   edge2_d=edge2.p[1]-edge2.p[0];
   Bool   swap=false;
   if(Dot(move2, Perp(edge2_d))>0)
   {
      Swap(edge2.p[0], edge2.p[1]);
      edge2_d.chs();
      swap=true;
   }
   Flt frac; Vec2 normal2; if(!SweepCircleEdge(Circle(capsule.r), move2, edge2, &frac, &normal2))
   {
      check=(DistPointEdge(capsule.pos+capsule.up*ihh, edge.p[0], edge.p[1])<DistPointEdge(capsule.pos-capsule.up*ihh, edge.p[0], edge.p[1]));
   }else
   {
      Byte hitd =false;
      Flt  pos_h=frac*Dot      (move                  , capsule.up),
           e0_h =DistPointPlane(edge.p[0], capsule.pos, capsule.up),
           e1_h =DistPointPlane(edge.p[1], capsule.pos, capsule.up);
      if(swap)Swap(e0_h, e1_h);
      if(Equal(edge2.p[0], edge2.p[1]))
      {
         Flt min_h, max_h; MinMax(e0_h, e1_h, min_h, max_h);
         if(min_h>pos_h+ihh)check=1;else // upper
         if(max_h<pos_h-ihh)check=0;else // lower
            hitd=true;
      }else
      {
         Vec2  hit_point=frac*move2 - capsule.r*normal2;
         Flt  edge_step =Sat(DistPointPlane(hit_point, edge2.p[0], edge2_d)/DistPointPlane(edge2.p[1], edge2.p[0], edge2_d));
         Flt  edge_h    =e0_h + edge_step*(e1_h-e0_h);
         if(Abs(edge_h-pos_h)>ihh)check=(edge_h>pos_h);else
            hitd=true;
      }
      if(hitd)
      {
         if(hit_frac  )*hit_frac  =frac;
         if(hit_normal)*hit_normal=matrix.orn().convert(normal2);
         return true;
      }
   }
   Ball ball(capsule.r, capsule.pos); ball.pos+=capsule.up*(check ? ihh : -ihh);
   return SweepBallEdge(ball, move, edge, hit_frac, hit_normal);
}
/******************************************************************************/
Bool SweepCapsulePlane(C Capsule &capsule, C Vec &move, C Plane &plane, Flt *hit_frac, Vec *hit_normal, Vec *hit_pos)
{
   if(Dot(move, plane.normal)>=0)return false;
   return SweepPointPlane(capsule.nearestNrm(plane.normal), move, plane, hit_frac, hit_normal, hit_pos);
}
Bool SweepCapsuleTri(C Capsule &capsule, C Vec &move, C TriN &tri, Flt *hit_frac, Vec *hit_normal)
{
   if(Dot(move, tri.n)>=0)return false;
   if(SweepPointTri(capsule.nearestNrm(tri.n), move, tri, hit_frac))
   {
      if(hit_normal)*hit_normal=tri.n;
      return true;
   }

   Byte hit=false;
   Flt  f, frac;
   Vec  n, normal;
   REP(3)if(SweepCapsuleEdge(capsule, move, tri.edge(i), &f, &n))if(!hit || f<frac){hit=true; frac=f; normal=n;}
   if(hit)
   {
      if(hit_frac  )*hit_frac  =frac  ;
      if(hit_normal)*hit_normal=normal;
      return true;
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
