/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'Capsule' to handle capsule shapes.

/******************************************************************************/
struct Capsule // Capsule Shape
{
   Flt r  , // radius
       h  ; // total  height
   Vec pos, // center position
       up ; // up     direction

   // set
   Capsule& set    (Flt r,   Flt  h   , C Vec &pos=VecZero, C Vec &up=Vec(0,1,0)) {T.r=r; T.h=h; T.pos=pos; T.up=up; return T;}
   Capsule& set    (Flt r, C Vec &from, C Vec &to); // 'from' and 'to' are the bottom and top points of capsule
   Capsule& setEdge(Flt r, C Vec &from, C Vec &to); // 'from' and 'to' are the bottom and top points of capsule inner edge

   // get
   Flt   areaCapsule()C {return PI2*r*   h            ;} // get surface area, can be used if you know that this is a capsule and not a ball "!isBall"
   Flt volumeCapsule()C {return PI *r*r*(h-r*(2.0f/3));} // get volume      , can be used if you know that this is a capsule and not a ball "!isBall"

   Flt   area()C {return isBall() ? (PI*4  )*Sqr (ballR()) :   areaCapsule();} // get surface area
   Flt volume()C {return isBall() ? (PI*4/3)*Cube(ballR()) : volumeCapsule();} // get volume

   Flt innerHeightHalf()C {return h*0.5f-r;}

   Vec  bottom  ()C {return pos-up*(h*0.5f);             } // get bottom
   Vec  top     ()C {return pos+up*(h*0.5f);             } // get top
   Vec  ballDPos()C {return pos-up*innerHeightHalf();    } // get lower ball center
   Vec  ballUPos()C {return pos+up*innerHeightHalf();    } // get upper ball center
   Ball ballD   ()C {return Ball(r, ballDPos());         } // get lower ball
   Ball ballU   ()C {return Ball(r, ballUPos());         } // get upper ball
   Edge fullEdge()C {return Edge(bottom  (), top     ());} // get edge between bottom and top
   Edge ballEdge()C {return Edge(ballDPos(), ballUPos());} // get edge between lower  and upper ball centers

   Vec nearestNrm(C Vec &normal          )C; // get nearest point on capsule towards normal
   Vec nearestPos(C Vec &pos, Bool inside)C; // get nearest point on capsule towards position, 'inside'=if allow retuning position inside capsule (false on surface only)

   Bool isBall   ()C {return h<=r*2        ;} // if this capsule is actually a ball (total height is equal or smaller than 2*radius)
   Flt  ballR    ()C {return     r         ;} // radius used for ball, can be used if you already know that 'isBall'
   Flt  ballRSafe()C {return Max(r, h*0.5f);} // radius used for ball

   Str asText()C {return S+"Radius: "+r+", Height: "+h+", Pos: "+pos+", Up: "+up;} // get text description

   // transform
   Capsule& operator+=(C Vec     &v) {pos+=v; return T;}
   Capsule& operator-=(C Vec     &v) {pos-=v; return T;}
   Capsule& operator*=(  Flt      f);
   Capsule& operator/=(  Flt      f);
   Capsule& operator*=(C Vec     &v);
   Capsule& operator/=(C Vec     &v);
   Capsule& operator*=(C Matrix3 &m);
   Capsule& operator/=(C Matrix3 &m);
   Capsule& operator*=(C Matrix  &m);
   Capsule& operator/=(C Matrix  &m);
   Capsule& operator*=(C MatrixM &m);
   Capsule& operator/=(C MatrixM &m);

   friend Capsule operator+ (C Capsule &capsule, C Vec     &v) {return Capsule(capsule)+=v;}
   friend Capsule operator- (C Capsule &capsule, C Vec     &v) {return Capsule(capsule)-=v;}
   friend Capsule operator* (C Capsule &capsule,   Flt      f) {return Capsule(capsule)*=f;}
   friend Capsule operator/ (C Capsule &capsule,   Flt      f) {return Capsule(capsule)/=f;}
   friend Capsule operator* (C Capsule &capsule, C Vec     &v) {return Capsule(capsule)*=v;}
   friend Capsule operator/ (C Capsule &capsule, C Vec     &v) {return Capsule(capsule)/=v;}
   friend Capsule operator* (C Capsule &capsule, C Matrix3 &m) {return Capsule(capsule)*=m;}
   friend Capsule operator/ (C Capsule &capsule, C Matrix3 &m) {return Capsule(capsule)/=m;}
   friend Capsule operator* (C Capsule &capsule, C Matrix  &m) {return Capsule(capsule)*=m;}
   friend Capsule operator/ (C Capsule &capsule, C Matrix  &m) {return Capsule(capsule)/=m;}

   // operations
   Capsule& extend(Flt e) {r+=e; h+=e+e; return T;} // extend

   // draw
   void draw(C Color &color=WHITE, Bool fill=false, Int resolution=-1)C; // this relies on active object matrix which can be set using 'SetMatrix' function

   Capsule() {}
   Capsule(Flt r,   Flt  h   , C Vec &pos=VecZero, C Vec &up=Vec(0,1,0)) {set(r, h, pos , up);}
   Capsule(Flt r, C Vec &from, C Vec &to                               ) {set(r,    from, to);}
};
/******************************************************************************/
struct CapsuleM // Capsule Shape (mixed precision)
{
   Flt  r  , // radius
        h  ; // total  height
   VecD pos; // center position
   Vec  up ; // up     direction

   // set
   CapsuleM& set(Flt r, Flt h, C VecD &pos=VecDZero, C Vec &up=Vec(0,1,0)) {T.r=r; T.h=h; T.pos=pos; T.up=up; return T;}

   // get
   Flt   areaCapsule()C {return PI2*r*   h            ;} // get surface area, can be used if you know that this is a capsule and not a ball "!isBall"
   Flt volumeCapsule()C {return PI *r*r*(h-r*(2.0f/3));} // get volume      , can be used if you know that this is a capsule and not a ball "!isBall"

   Flt   area()C {return isBall() ? (PI*4  )*Sqr (ballR()) :   areaCapsule();} // get surface area
   Flt volume()C {return isBall() ? (PI*4/3)*Cube(ballR()) : volumeCapsule();} // get volume

   Flt innerHeightHalf()C {return h*0.5f-r;}

   VecD  bottom  ()C {return pos-up*(h*0.5f);              } // get bottom
   VecD  top     ()C {return pos+up*(h*0.5f);              } // get top
   VecD  ballDPos()C {return pos-up*innerHeightHalf();     } // get lower ball center
   VecD  ballUPos()C {return pos+up*innerHeightHalf();     } // get upper ball center
   BallM ballD   ()C {return BallM(r, ballDPos());         } // get lower ball
   BallM ballU   ()C {return BallM(r, ballUPos());         } // get upper ball
   EdgeD fullEdge()C {return EdgeD(bottom  (), top     ());} // get edge between bottom and top
   EdgeD ballEdge()C {return EdgeD(ballDPos(), ballUPos());} // get edge between lower  and upper ball centers

   VecD nearestNrm(C Vec  &normal          )C; // get nearest point on capsule towards normal
   VecD nearestPos(C VecD &pos, Bool inside)C; // get nearest point on capsule towards position, 'inside'=if allow retuning position inside capsule (false on surface only)

   Bool isBall   ()C {return h<=r*2        ;} // if this capsule is actually a ball (total height is equal or smaller than 2*radius)
   Flt  ballR    ()C {return     r         ;} // radius used for ball, can be used if you already know that 'isBall'
   Flt  ballRSafe()C {return Max(r, h*0.5f);} // radius used for ball

   Str asText()C {return S+"Radius: "+r+", Height: "+h+", Pos: "+pos+", Up: "+up;} // get text description

   // transform
   CapsuleM& operator+=(C VecD    &v) {pos+=v; return T;}
   CapsuleM& operator-=(C VecD    &v) {pos-=v; return T;}
   CapsuleM& operator*=(  Flt      f);
   CapsuleM& operator/=(  Flt      f);
   CapsuleM& operator*=(C Vec     &v);
   CapsuleM& operator/=(C Vec     &v);
   CapsuleM& operator*=(C Matrix3 &m);
   CapsuleM& operator/=(C Matrix3 &m);
   CapsuleM& operator*=(C Matrix  &m);
   CapsuleM& operator/=(C Matrix  &m);
   CapsuleM& operator*=(C MatrixM &m);
   CapsuleM& operator/=(C MatrixM &m);

   friend CapsuleM operator+ (C CapsuleM &capsule, C VecD    &v) {return CapsuleM(capsule)+=v;}
   friend CapsuleM operator- (C CapsuleM &capsule, C VecD    &v) {return CapsuleM(capsule)-=v;}
   friend CapsuleM operator* (C CapsuleM &capsule,   Flt      f) {return CapsuleM(capsule)*=f;}
   friend CapsuleM operator/ (C CapsuleM &capsule,   Flt      f) {return CapsuleM(capsule)/=f;}
   friend CapsuleM operator* (C CapsuleM &capsule, C Vec     &v) {return CapsuleM(capsule)*=v;}
   friend CapsuleM operator/ (C CapsuleM &capsule, C Vec     &v) {return CapsuleM(capsule)/=v;}
   friend CapsuleM operator* (C CapsuleM &capsule, C Matrix3 &m) {return CapsuleM(capsule)*=m;}
   friend CapsuleM operator/ (C CapsuleM &capsule, C Matrix3 &m) {return CapsuleM(capsule)/=m;}
   friend CapsuleM operator* (C CapsuleM &capsule, C Matrix  &m) {return CapsuleM(capsule)*=m;}
   friend CapsuleM operator/ (C CapsuleM &capsule, C Matrix  &m) {return CapsuleM(capsule)/=m;}
   friend CapsuleM operator* (C CapsuleM &capsule, C MatrixM &m) {return CapsuleM(capsule)*=m;}
   friend CapsuleM operator/ (C CapsuleM &capsule, C MatrixM &m) {return CapsuleM(capsule)/=m;}

              CapsuleM() {}
              CapsuleM(Flt r, Flt h, C VecD &pos=VecDZero, C Vec &up=Vec(0,1,0)) {set(  r,   h,   pos,   up);}
   CONVERSION CapsuleM(C Capsule &c                                            ) {set(c.r, c.h, c.pos, c.up);}
};
/******************************************************************************/
inline CapsuleM operator* (C Capsule &capsule, C MatrixM &m) {return CapsuleM(capsule)*=m;}
inline CapsuleM operator/ (C Capsule &capsule, C MatrixM &m) {return CapsuleM(capsule)/=m;}

// distance
       Flt Dist            (C Vec      &point  , C Capsule  &capsule             ); // distance between point    and a capsule
       Dbl Dist            (C VecD     &point  , C CapsuleM &capsule             ); // distance between point    and a capsule
       Flt Dist            (C Edge     &edge   , C Capsule  &capsule             ); // distance between edge     and a capsule
       Dbl Dist            (C EdgeD    &edge   , C CapsuleM &capsule             ); // distance between edge     and a capsule
       Flt Dist            (C TriN     &tri    , C Capsule  &capsule             ); // distance between triangle and a capsule
       Flt Dist            (C Box      &box    , C Capsule  &capsule             ); // distance between box      and a capsule
       Flt Dist            (C OBox     &obox   , C Capsule  &capsule             ); // distance between box      and a capsule
       Flt Dist            (C Ball     &ball   , C Capsule  &capsule             ); // distance between ball     and a capsule
       Flt Dist            (C Capsule  &a      , C Capsule  &b                   ); // distance between capsule  and a capsule
       Flt DistFull        (C Capsule  &a      , C Capsule  &b                   ); // distance between capsule  and a capsule, can return negative values if shapes intersect
       Flt DistCapsulePlane(C Capsule  &capsule, C Vec      &plane, C Vec &normal); // distance between capsule  and a plane
       Dbl DistCapsulePlane(C Capsule  &capsule, C VecD     &plane, C Vec &normal); // distance between capsule  and a plane
       Dbl DistCapsulePlane(C CapsuleM &capsule, C VecD     &plane, C Vec &normal); // distance between capsule  and a plane
inline Flt Dist            (C Capsule  &capsule, C Plane    &plane               ) {return DistCapsulePlane(capsule, plane.pos, plane.normal);} // distance between capsule and a plane
inline Dbl Dist            (C Capsule  &capsule, C PlaneM   &plane               ) {return DistCapsulePlane(capsule, plane.pos, plane.normal);} // distance between capsule and a plane
inline Dbl Dist            (C CapsuleM &capsule, C PlaneM   &plane               ) {return DistCapsulePlane(capsule, plane.pos, plane.normal);} // distance between capsule and a plane

// cuts
Bool Cuts(C Vec     &point, C Capsule  &capsule); // if point    cuts a capsule
Bool Cuts(C VecD    &point, C Capsule  &capsule); // if point    cuts a capsule
Bool Cuts(C VecD    &point, C CapsuleM &capsule); // if point    cuts a capsule
Bool Cuts(C Edge    &edge , C Capsule  &capsule); // if edge     cuts a capsule
Bool Cuts(C Tri     &tri  , C Capsule  &capsule); // if triangle cuts a capsule
Bool Cuts(C TriN    &tri  , C Capsule  &capsule); // if triangle cuts a capsule
Bool Cuts(C Box     &box  , C Capsule  &capsule); // if box      cuts a capsule
Bool Cuts(C OBox    &obox , C Capsule  &capsule); // if box      cuts a capsule
Bool Cuts(C Ball    &ball , C Capsule  &capsule); // if ball     cuts a capsule
Bool Cuts(C BallM   &ball , C Capsule  &capsule); // if ball     cuts a capsule
Bool Cuts(C BallM   &ball , C CapsuleM &capsule); // if ball     cuts a capsule
Bool Cuts(C Capsule &a    , C Capsule  &b      ); // if capsule  cuts a capsule

// sweep
Bool SweepPointCapsule(C Vec  &point, C Vec &move, C Capsule  &capsule, Flt *hit_frac=null, Vec *hit_normal=null); // if moving point cuts through a static capsule
Bool SweepPointCapsule(C VecD &point, C Vec &move, C CapsuleM &capsule, Flt *hit_frac=null, Vec *hit_normal=null); // if moving point cuts through a static capsule
Bool SweepBallCapsule (C Ball &ball , C Vec &move, C Capsule  &capsule, Flt *hit_frac=null, Vec *hit_normal=null); // if moving ball  cuts through a static capsule

Bool SweepCapsuleEdge (C Capsule &capsule, C Vec &move, C Edge  &edge , Flt *hit_frac=null, Vec *hit_normal=null                   ); // if moving capsule cuts through a static edge
Bool SweepCapsulePlane(C Capsule &capsule, C Vec &move, C Plane &plane, Flt *hit_frac=null, Vec *hit_normal=null, Vec *hit_pos=null); // if moving capsule cuts through a static plane
Bool SweepCapsuleTri  (C Capsule &capsule, C Vec &move, C TriN  &tri  , Flt *hit_frac=null, Vec *hit_normal=null                   ); // if moving capsule cuts through a static triangle
/******************************************************************************/
