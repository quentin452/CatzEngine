/******************************************************************************

   Use 'Camera' to handle setting the viewing camera,
      where you can use the default 'Cam' camera, or create your own 'Camera' objects.
   Use 'ActiveCam' to access properties of currently active camera.

   Use helper functions to convert shapes between 3D world space and 2D screen space.

/******************************************************************************/
enum CAMH_FLAG // Camera Handle flags
{
   CAMH_ZOOM        =1<<0, // zoom   with mouse wheel
   CAMH_ROT_X       =1<<1, // rotate.x on mouse move (yaw)
   CAMH_ROT_Y       =1<<2, // rotate.y on mouse move (pitch)
   CAMH_MOVE        =1<<3, // move     on mouse move
   CAMH_MOVE_XZ     =1<<4, // move.xz  on mouse move
   CAMH_NO_BEGIN    =1<<5, // don't automatically call 'updateBegin'
   CAMH_NO_END      =1<<6, // don't automatically call 'updateEnd'
   CAMH_NO_SET      =1<<7, // don't automatically call 'set' to activate the camera
   CAMH_ROT         =CAMH_ROT_X|CAMH_ROT_Y    , // rotate on mouse move
   CAMH_NO_BEGIN_END=CAMH_NO_BEGIN|CAMH_NO_END, // don't automatically call 'updateBegin, updateEnd'
};
/******************************************************************************/
struct Camera
{
   Flt     yaw   , // yaw   angle                                    , default=0
           pitch , // pitch angle                                    , default=0
           roll  , // roll  angle                                    , default=0
           dist  ; // distance between camera position and 'at' point, default=1
   VecD    at    ; // point where camera is looking at               , default=VecD(0,0,0)
   MatrixM matrix; // camera object matrix                           , default=MatrixIdentity-Vec(0,0,1)

   Vec        vel, // camera         velocity (this gets modified when calling 'updateEnd')
          ang_vel; // camera angular velocity (this gets modified when calling 'updateEnd')

   Camera& operator+=(C VecD &offset) {at+=offset; matrix.pos+=offset; return T;} // move camera by 'offset'
   Camera& operator-=(C VecD &offset) {at-=offset; matrix.pos-=offset; return T;} // move camera by 'offset'

   // update
      // begin
      Camera& updateBegin(); // call this once per frame, before changing camera matrix

      // set
      Camera& set         (C MatrixM &matrix                                            ); // set from matrix
      Camera& setAngle    (C VecD    &pos , Flt yaw, Flt pitch, Flt roll=0              ); // set from position and angles
      Camera& setSpherical(C VecD    &at  , Flt yaw, Flt pitch, Flt roll, Flt dist      ); // set spherical from 'look at' position, angles and distance
      Camera& setSpherical(                                                             ); // set 'matrix' member as a spherical camera from current 'at dist yaw pitch roll' member values
      Camera& setFromAt   (C VecD    &from, C VecD &at            ,   Flt roll=0        ); // set from "look from" position, "look at" position and 'roll' angle
      Camera& setPosDir   (C VecD    &pos , C Vec  &dir=Vec(0,0,1), C Vec &up=Vec(0,1,0)); // set from look directions

      // end
      Camera& updateEnd(); // call this once per frame, after changing camera matrix

   // operations
#if EE_PRIVATE
   Camera& teleported();
#endif
   void set()C; // set as active camera - sets rendering matrixes, sets frustum, copies self to 'ActiveCam'

   Camera& transformByMouse(Flt dist_min, Flt dist_max, UInt flag); // this is a helper method that transforms the camera basing on mouse input, 'dist_min'=minimum zoom distance, 'dist_max'=maximum zoom distance, 'flag'=CAMH_FLAG

   // io
   Bool save(File &f)C; // save to   file, false on fail
   Bool load(File &f) ; // load from file, false on fail

   Camera();

#if !EE_PRIVATE
private:
#endif
   MatrixM _matrix_prev;
   VecD    _pos_prev1;
};
extern   Camera       Cam; // default camera, you can use it to manipulate the camera
extern C Camera ActiveCam; // active  camera in read-only mode, this object is always changed when you activate any 'Camera' using its 'set' method, when rendering mirror/reflections, its 'matrix' temporarily gets adjusted
/******************************************************************************/
//
// Following functions work on active viewport
//
Vec2 PosToScreen(C Vec  &pos, C MatrixM &camera_matrix              ); // convert 3D position to 2D screen position
Vec2 PosToScreen(C VecD &pos, C MatrixM &camera_matrix              ); // convert 3D position to 2D screen position
Bool PosToScreen(C Vec  &pos, C MatrixM &camera_matrix, Vec2 &screen); // convert 3D position to 2D screen position, false on fail (point is behind the camera)
Bool PosToScreen(C VecD &pos, C MatrixM &camera_matrix, Vec2 &screen); // convert 3D position to 2D screen position, false on fail (point is behind the camera)

//
// Following functions work on active viewport and 'ActiveCam' camera:
//
Vec2 PosToScreen (C Vec  &pos              ); // convert 3D position to 2D screen position
Vec2 PosToScreen (C VecD &pos              ); // convert 3D position to 2D screen position
Vec2 PosToScreenM(C Vec  &pos              ); // convert 3D position to 2D screen position (transformed by current object matrix)
Vec2 PosToScreenM(C VecD &pos              ); // convert 3D position to 2D screen position (transformed by current object matrix)
Bool PosToScreen (C Vec  &pos, Vec2 &screen); // convert 3D position to 2D screen position                                       , false on fail (point is behind the camera)
Bool PosToScreen (C VecD &pos, Vec2 &screen); // convert 3D position to 2D screen position                                       , false on fail (point is behind the camera)
Bool PosToScreenM(C Vec  &pos, Vec2 &screen); // convert 3D position to 2D screen position (transformed by current object matrix), false on fail (point is behind the camera)
Bool PosToScreenM(C VecD &pos, Vec2 &screen); // convert 3D position to 2D screen position (transformed by current object matrix), false on fail (point is behind the camera)

Vec  ScreenToPosD   (C Vec2 &screen_d, Flt z=ActiveCam.dist); // convert 2D screen delta    to 3D world-space delta    (affected by camera)
VecD ScreenToPos    (C Vec2 &screen  , Flt z=ActiveCam.dist); // convert 2D screen position to 3D world-space position (affected by camera)
Vec  ScreenToViewPos(C Vec2 &screen  , Flt z=ActiveCam.dist); // convert 2D screen position to 3D  view-space position (in camera space)
Vec  ScreenToPosDM  (C Vec2 &screen_d, Flt z=ActiveCam.dist); // convert 2D screen delta    to 3D world-space delta    (affected by camera, transformed by current object drawing matrix which is set by 'SetMatrix')
VecD ScreenToPosM   (C Vec2 &screen  , Flt z=ActiveCam.dist); // convert 2D screen position to 3D world-space position (affected by camera, transformed by current object drawing matrix which is set by 'SetMatrix')

Vec  ScreenToDir   (C Vec2 &screen                     ); // convert 2D screen position to world space                       3D direction
void ScreenToPosDir(C Vec2 &screen, Vec  &pos, Vec &dir); // convert 2D screen position to world space 3D position start and 3D direction
void ScreenToPosDir(C Vec2 &screen, VecD &pos, Vec &dir); // convert 2D screen position to world space 3D position start and 3D direction

Bool ToScreenRect(C Box      &box    ,             Rect &rect); // project 3D box     to 2D on-screen rectangle, false on fail (shape is behind the camera)
Bool ToScreenRect(C OBox     &obox   ,             Rect &rect); // project 3D obox    to 2D on-screen rectangle, false on fail (shape is behind the camera)
Bool ToScreenRect(C Ball     &ball   ,             Rect &rect); // project 3D ball    to 2D on-screen rectangle, false on fail (shape is behind the camera)
Bool ToScreenRect(C BallM    &ball   ,             Rect &rect); // project 3D ball    to 2D on-screen rectangle, false on fail (shape is behind the camera)
Bool ToScreenRect(C Capsule  &capsule,             Rect &rect); // project 3D capsule to 2D on-screen rectangle, false on fail (shape is behind the camera)
Bool ToScreenRect(C CapsuleM &capsule,             Rect &rect); // project 3D capsule to 2D on-screen rectangle, false on fail (shape is behind the camera)
Bool ToScreenRect(C Pyramid  &pyramid,             Rect &rect); // project 3D pyramid to 2D on-screen rectangle, false on fail (shape is behind the camera)
Bool ToScreenRect(C Shape    &shape  ,             Rect &rect); // project 3D shape   to 2D on-screen rectangle, false on fail (shape is behind the camera)
Bool ToScreenRect(C Shape    *shape  , Int shapes, Rect &rect); // project 3D shapes  to 2D on-screen rectangle, false on fail (shape is behind the camera)

Int CompareTransparencyOrderDepth(C Vec  &pos_a, C Vec  &pos_b); // return comparing value -1/0/+1 determining the order of transparent objects rendering according to their depth    (linear    distance along camera look direction)
Int CompareTransparencyOrderDepth(C VecD &pos_a, C VecD &pos_b); // return comparing value -1/0/+1 determining the order of transparent objects rendering according to their depth    (linear    distance along camera look direction)
Int CompareTransparencyOrderDist (C Vec  &pos_a, C Vec  &pos_b); // return comparing value -1/0/+1 determining the order of transparent objects rendering according to their distance (spherical distance from  camera position)
Int CompareTransparencyOrderDist (C VecD &pos_a, C VecD &pos_b); // return comparing value -1/0/+1 determining the order of transparent objects rendering according to their distance (spherical distance from  camera position)

#if EE_PRIVATE
Vec2 PosToFullScreen(C Vec  &pos              ); // convert 3D position to 2D (full and not gui) screen position
Vec2 PosToFullScreen(C VecD &pos              ); // convert 3D position to 2D (full and not gui) screen position
Bool PosToFullScreen(C Vec  &pos, Vec2 &screen); // convert 3D position to 2D (full and not gui) screen position, false on fail (point is behind the camera)
Bool PosToFullScreen(C VecD &pos, Vec2 &screen); // convert 3D position to 2D (full and not gui) screen position, false on fail (point is behind the camera)

VecD FullScreenToPos    (C Vec2 &screen, Flt z=ActiveCam.dist); // convert 2D (full and not gui) screen position to 3D world-space position (affected by camera)
Vec  FullScreenToViewPos(C Vec2 &screen, Flt z=ActiveCam.dist); // convert 2D (full and not gui) screen position to 3D  view-space position (in camera space)

Bool ToFullScreenRect(C Ball     &ball   , Rect &rect); // project 3D ball    to 2D (full and not gui) on-screen rectangle, false on fail (shape is behind the camera)
Bool ToFullScreenRect(C BallM    &ball   , Rect &rect); // project 3D ball    to 2D (full and not gui) on-screen rectangle, false on fail (shape is behind the camera)
Bool ToFullScreenRect(C Pyramid  &pyramid, Rect &rect); // project 3D pyramid to 2D (full and not gui) on-screen rectangle, false on fail (shape is behind the camera)
Bool ToFullScreenRect(C PyramidM &pyramid, Rect &rect); // project 3D pyramid to 2D (full and not gui) on-screen rectangle, false on fail (shape is behind the camera)
#endif
/******************************************************************************/
#if EE_PRIVATE
void InitCamera   ();
void  SetCam      (C MatrixM &matrix                        ); // 'Frustum' may have to be reset after calling this method
void  SetCam      (C MatrixM &matrix, C MatrixM &matrix_prev); // 'Frustum' may have to be reset after calling this method
void  SetEyeMatrix();
void  ActiveCamChanged();
void  SetViewToViewPrev();

extern Dbl ActiveCamZ;

inline Dbl DistPointActiveCamPlaneZ(C VecD &point) {return Dot(point, ActiveCam.matrix.z)-ActiveCamZ;}
#endif
/******************************************************************************/
