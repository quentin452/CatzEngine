/******************************************************************************

   Use 'AnimBone'  to store animation keys for a particular bone.
   Use 'AnimEvent' to store information about a custom event in the animation at a custom time moment.
   Use 'Animation' to store body animation keys, animation keys for multiple bones, and events.
   Use 'SkelAnim'  to handle the relation between a skeleton and an animation.

/******************************************************************************/
struct AnimBone : AnimKeys, BoneID // Animation Bone - set of animation keyframes for a particular bone
{
   void save(TextNode &node)C; // save as text
};
/******************************************************************************/
struct AnimEvent // Animation Event - custom event which occurs at 'time' in 'Animation'
{
   Char8 name[32]; // name of the event
   Flt   time=0  ; // time position when the event occurs

   AnimEvent& set(C Str8 &name, Flt time) {Set(T.name, name); T.time=time; return T;}

   void save(TextNode &node)C; // save as text

   AnimEvent() {name[0]=0;}

   Bool operator==(C AnimEvent &event)C;
};
/******************************************************************************/
enum ANIM_FLAG // Animation Flags
{
   ANIM_LINEAR=0x1, // linear (without smoothing)
   ANIM_LOOP  =0x2, // looped
};
enum ROOT_FLAG // Root Flags
{
   ROOT_BONE_POSITION =1<< 0, // set    root animation position from bone position (valid only when converting bone to root, ignored if root already available)
   ROOT_START_IDENTITY=1<< 1, // start  root animation with identity
   ROOT_DEL_POSITION_X=1<< 2, // remove root animation position X
   ROOT_DEL_POSITION_Y=1<< 3, // remove root animation position Y
   ROOT_DEL_POSITION_Z=1<< 4, // remove root animation position Z
   ROOT_DEL_ROTATION_X=1<< 5, // remove root animation rotation X
   ROOT_DEL_ROTATION_Y=1<< 6, // remove root animation rotation Y
   ROOT_DEL_ROTATION_Z=1<< 7, // remove root animation rotation Z
   ROOT_DEL_SCALE     =1<< 8, // remove root animation scale
   ROOT_SMOOTH_ROT    =1<< 9, // set    root animation rotation to be smooth with constant velocities
   ROOT_SMOOTH_POS    =1<<10, // set    root animation position to be smooth with constant velocities
   ROOT_LINEAR_POS    =1<<11, // set    root animation position to be linear with constant velocities and have only 2 keyframes: start+end
   ROOT_SMOOTH_SCALE  =1<<12, // set    root animation scale    to be smooth with constant velocities

   ROOT_DEL_POSITION  =ROOT_DEL_POSITION_X|ROOT_DEL_POSITION_Y|ROOT_DEL_POSITION_Z,
   ROOT_DEL_ROTATION  =ROOT_DEL_ROTATION_X|ROOT_DEL_ROTATION_Y|ROOT_DEL_ROTATION_Z,
   ROOT_DEL           =ROOT_DEL_POSITION|ROOT_DEL_ROTATION|ROOT_DEL_SCALE,
   ROOT_SMOOTH_ROT_POS=ROOT_SMOOTH_ROT|ROOT_SMOOTH_POS,
   ROOT_SMOOTH        =ROOT_SMOOTH_ROT_POS|ROOT_SMOOTH_SCALE,
};
/******************************************************************************/
struct Animation // set of animation keyframes used for animating 'AnimatedSkeleton'
{
   Mems<AnimBone>  bones ; // bone animations
   Mems<AnimEvent> events; // animation events, sorted by time
   AnimKeys        keys  ; // animation keys of the whole body

   // get / set
   Bool is()C {return bones.elms() || events.elms() || keys.is();} // if has any data

   Int       findBoneI(CChar8 *name, BONE_TYPE type=BONE_UNKNOWN, Int type_index=0, Int type_sub=0)C; // find bone animation index, bones are first compared against the specified 'name', if there's no exact match and the 'type' is specified (not BONE_UNKNOWN) then type parameters are used for searching, -1   on fail
   AnimBone* findBone (CChar8 *name, BONE_TYPE type=BONE_UNKNOWN, Int type_index=0, Int type_sub=0) ; // find bone animation      , bones are first compared against the specified 'name', if there's no exact match and the 'type' is specified (not BONE_UNKNOWN) then type parameters are used for searching, null on fail
 C AnimBone* findBone (CChar8 *name, BONE_TYPE type=BONE_UNKNOWN, Int type_index=0, Int type_sub=0)C; // find bone animation      , bones are first compared against the specified 'name', if there's no exact match and the 'type' is specified (not BONE_UNKNOWN) then type parameters are used for searching, null on fail
   AnimBone&  getBone (CChar8 *name, BONE_TYPE type=BONE_UNKNOWN, Int type_index=0, Int type_sub=0) ; // get  bone animation      , bones are first compared against the specified 'name', if there's no exact match and the 'type' is specified (not BONE_UNKNOWN) then type parameters are used for searching, creates new bone animation if not found

   Animation& loop  (Bool loop                          );   Bool loop  ()C {return  FlagOn(_flag, ANIM_LOOP  );} // set/get looping
   Animation& linear(Bool linear                        );   Bool linear()C {return  FlagOn(_flag, ANIM_LINEAR);} // set/get linear smoothing (setting linear smoothing makes the animation look more mechanized)
   Animation& length(Flt  length, Bool rescale_keyframes);   Flt  length()C {return _length                    ;} // set/get animation length, 'rescale_keyframes'=if proportionally rescale keyframes from current length to new length

   AnimEvent* findEvent(CChar8 *name) ; // find animation event, null if not found
 C AnimEvent* findEvent(CChar8 *name)C; // find animation event, null if not found

   Int eventCount(CChar8 *name)C; // get number of events with specified name in this animation

   Bool eventAfter   (CChar8 *name            , Flt       time        )C; // if after  'name' event                     ,       'time'=animation time
   Bool eventBefore  (CChar8 *name            , Flt       time        )C; // if before 'name' event                     ,       'time'=animation time
   Bool eventOccurred(CChar8 *name            , Flt start_time, Flt dt)C; // if        'name' event       occurred      , 'start_time'=animation time at the start of the frame, 'dt'=animation time delta (should be set to "Time.d() * animation_speed")
   Int eventsOccurred(Int    &first_event     , Flt start_time, Flt dt)C; // get number of    events that occurred      , 'start_time'=animation time at the start of the frame, 'dt'=animation time delta (should be set to "Time.d() * animation_speed"), returns number of events that occurred in specified time range, 'first_event'=index of the first event, event indexes should be calculated like this: "(first_event+i)%Animation.events()" where 'i' is event index "0 .. 'eventsOccurred'-1"
   Bool eventBetween (CChar8 *from, CChar8 *to, Flt start_time, Flt dt)C; // if           between events 'from' and 'to', 'start_time'=animation time at the start of the frame, 'dt'=animation time delta (should be set to "Time.d() * animation_speed")
   Flt  eventProgress(CChar8 *from, CChar8 *to, Flt       time        )C; // get progress between events 'from' and 'to',       'time'=animation time, 0 on fail

   void getRootMatrix          (   Matrix &matrix   , Flt time                    )C; // get root 'matrix' at specified 'time'
   void getRootMatrixCumulative(   Matrix &matrix   , Flt time                    )C; // get root 'matrix' at specified 'time' with accumulation enabled for looped animations
   void getRootTransform       (RevMatrix &transform, Flt start_time, Flt end_time)C; // get matrix that transforms root from 'start_time' to 'end_time'

 C    Matrix& rootStart    ()C {return _root_start    ;} // get root matrix at the start of animation
 C    Matrix& rootEnd      ()C {return _root_end      ;} // get root matrix at the end   of animation
 C RevMatrix& rootTransform()C {return _root_transform;} // get      matrix that transforms 'rootStart' into 'rootEnd'

   Flt rootSpeed ()C {return _root_transform.pos.length ()/    _length ;} // get root   movement speed         between first and last keyframe
   Flt rootSpeed2()C {return _root_transform.pos.length2()/Sqr(_length);} // get root   movement speed squared between first and last keyframe
   Flt rootSpeedZ()C {return _root_transform.pos.z        /    _length ;} // get root Z movement speed         between first and last keyframe

   // transform
   Animation& transform(C Matrix &matrix, C Skeleton &source, Bool skel_const); // transform animation by 'matrix', basing on 'source' skeleton, 'skel_const'=if skeleton remains constant (was not and will not going to be transformed by 'matrix'), set to true if you haven't and won't transform the skeleton by 'matrix' or set to false if you have or will transform the skeleton by 'matrix'
   Animation& mirror   (                  C Skeleton &source                 ); // mirror    animation in X axis  , basing on 'source' skeleton

   // operations
   Animation& setTangents  (); // recalculate tangents     , this needs to be called after manually modifying keyframes
   Animation& setRootMatrix(); // recalculate root matrixes, this needs to be called after manually modifying keyframes

   Animation& optimize(Flt angle_eps=EPS_ANIM_ANGLE, Flt pos_eps=EPS_ANIM_POS, Flt scale_eps=EPS_ANIM_SCALE, Bool remove_unused_bones=true); // optimize animation by removing similar keyframes, 'angle_eps'=angular epsilon 0..PI, 'pos_eps'=position epsilon 0..Inf, 'scale_eps'=scale epsilon 0..Inf, 'remove_unused_bones'=if remove unused bones after performing key reduction

   Animation& clip          (Flt start_time, Flt end_time, Bool remove_unused_bones=true); // clip animation to 'start_time' .. 'end_time', this will remove all keyframes which aren't located in selected range
   Animation& clipAuto      (); // clip     animation range starting with first keyframe and ending with last keyframe
   Animation& maximizeLength(); // maximize animation length based on maximum time value out of all keyframes and events
   Animation& offsetTime    (Flt dt); // offset time values of keyframes
   Animation&  scaleTime    (Flt start_time, Flt end_time, Flt scale); // scale time values of keyframes that are between 'start_time' and 'end_time' by 'scale' factor

   Animation& adjustForSameSkeletonWithDifferentPose     (C Skeleton &source, C Skeleton &target                                                                                             ); // adjust animation which was created for 'source' skeleton to be used for 'target' skeleton
   Animation& adjustForSameTransformWithDifferentSkeleton(C Skeleton &source, C Skeleton &target, Int source_bone_as_root=-1, C CMemPtr< Mems<IndexWeight> > &weights=null, UInt root_flags=0); // adjust animation which was created for 'source' skeleton to be used for 'target' skeleton, 'source_bone_as_root'=index of a bone in 'source' skeleton that is converted to root, 'root_flags'=ROOT_FLAG, it's recommended to call 'optimize' after this method, because it may generate a lot of unnecessary keyframes

   Animation& offsetRootBones(C Skeleton &skeleton, C Vec &move); // offset all bones that have no parents by 'move' movement in global space

   Animation& setBoneTypeIndexesFromSkeleton    (C Skeleton &skeleton); // set bone animation      type and indexes from 'skeleton' according to bone animation name
   Bool       setBoneNameTypeIndexesFromSkeleton(C Skeleton &skeleton); // set bone animation name type and indexes from 'skeleton' according to bone animation info, returns true if any change was made

   Animation& reverse(); // reverse animation

   Animation& removeUnused(); // remove unused bone animations

   Animation& sortEvents(); // sort events in time order, this should be called after manually modifying the events and changing their time values
#if EE_PRIVATE
   void setRootMatrix2();
   void getRootMatrixExactTime(Matrix &matrix, Flt time)C; // get root 'matrix' at specified 'time'

   Bool timeRange(Flt &min, Flt &max)C; // get min/max time value out of all keyframes/events, false on fail (if there are no keyframes/events)

   Animation& sortFrames(); // sort frames in time order, this should be called after manually modifying keyframes and changing their time values

   void includeTimesForBoneAndItsParents(C Skeleton &skel, Int skel_bone, MemPtr<Flt, 16384> orn_times, MemPtr<Flt, 16384> pos_times, MemPtr<Flt, 16384> scale_times)C;

   Animation& copyParams(C Animation &src);

   // transform
   Animation& scale      (Flt scale         ); // scale position offset key values by 'scale'
   Animation& mirrorX    (                  ); // mirror keyframes in x axis
   Animation& rightToLeft(C Skeleton &source); // convert from right hand to left hand coordinate system, basing on 'source' skeleton and 'mesh' source mesh

   // convert
   Animation& convertRotToOrn(C Skeleton &skeleton); // convert relative  rotations to target orientations according to given 'skeleton'
   Animation& convertOrnToRot(C Skeleton &skeleton); // convert target orientations to relative  rotations according to given 'skeleton'
#endif

   void freezeBone(C Skeleton &skel, Int skel_bone); // adjust the animation by moving root bones, so that selected bone will appear without movement

   void freezeDelPos(C Skeleton &skel, Int skel_bone, Int key_index                             ); // delete 'key_index'    position key (use -1 for all keys) for 'skel_bone' (use -1 for root) in 'skel' skeleton without affecting transforms of other bones
   void freezeDelRot(C Skeleton &skel, Int skel_bone, Int key_index                   , Bool pos); // delete 'key_index' orientation key (use -1 for all keys) for 'skel_bone' (use -1 for root) in 'skel' skeleton without affecting transforms of other bones
   void freezeMove  (C Skeleton &skel, Int skel_bone, Int key_index, C Vec     &delta           ); // move   'key_index'    position key (use -1 for all keys) for 'skel_bone' (use -1 for root) in 'skel' skeleton without affecting transforms of other bones
   void freezeRotate(C Skeleton &skel, Int skel_bone, Int key_index, C Matrix3 &matrix, Bool pos); // rotate 'key_index' orientation key (use -1 for all keys) for 'skel_bone' (use -1 for root) in 'skel' skeleton without affecting transforms of other bones, 'matrix'=must be normalized

   // io
   void operator=(C Str &name) ; // load, Exit  on fail
   void operator=(C UID &id  ) ; // load, Exit  on fail
   Bool save     (C Str &name)C; // save, false on fail
   Bool load     (C Str &name) ; // load, false on fail

   Bool save(File &f)C; // save, false on fail
   Bool load(File &f) ; // load, false on fail

   void save(MemPtr<TextNode> nodes)C; // save as text

   Animation& del(); // delete manually
   Animation();

private:
        Byte _flag;
         Flt _length;
      Matrix _root_start, _root_end, _root_start_inv;
   RevMatrix _root_transform;
#if EE_PRIVATE
   void zero();
#endif
};
/******************************************************************************/
extern Cache<Animation> Animations; // Animation Cache
/******************************************************************************/
struct SkelAnim // helper class for 'Skeleton' <-> 'Animation' relation, 'SkelAnim' objects are by default obtained by 'AnimatedSkeleton.findSkelAnim' and 'AnimatedSkeleton.getSkelAnim'
{
   // manage
   SkelAnim& create(C Skeleton &skeleton, C Animation &animation); // create object to be used for 'skeleton' and 'animation' pair, 'animation' must point to constant memory address (only pointer to it is stored through which the object can be later accessed)

   // get
   Bool       is       ()C {return _animation!=null                     ;} // get if created
 C Animation* animation()C {return _animation                           ;} // get animation object
   Flt        length   ()C {return _animation ? _animation->length() : 0;} // get animation length
   CChar*     name     ()C {return  Animations.name(_animation         );} // get animation name
   UID        id       ()C {return  Animations.id  (_animation         );} // get animation name ID

   Int eventCount(CChar8 *name)C {return _animation ? _animation->eventCount(name) : 0;} // get number of events with specified name in this animation

   Bool eventAfter   (CChar8 *name            , Flt       time        )C {return _animation ? _animation->eventAfter    (name       ,       time    ) : false;} // if after  'name' event                     ,       'time'=animation time
   Bool eventBefore  (CChar8 *name            , Flt       time        )C {return _animation ? _animation->eventBefore   (name       ,       time    ) : false;} // if before 'name' event                     ,       'time'=animation time
   Bool eventOccurred(CChar8 *name            , Flt start_time, Flt dt)C {return _animation ? _animation->eventOccurred (name       , start_time, dt) : false;} // if        'name' event occurred            , 'start_time'=animation time at the start of the frame, 'dt'=animation time delta (should be set to "Time.d() * animation_speed")
   Int eventsOccurred(Int    &first_event     , Flt start_time, Flt dt)C {return _animation ? _animation->eventsOccurred(first_event, start_time, dt) :     0;} // get number of    events that occurred      , 'start_time'=animation time at the start of the frame, 'dt'=animation time delta (should be set to "Time.d() * animation_speed"), returns number of events that occurred in specified time range, 'first_event'=index of the first event, event indexes should be calculated like this: "(first_event+i)%Animation.events()" where 'i' is event index "0 .. 'eventsOccurred'-1"
   Bool eventBetween (CChar8 *from, CChar8 *to, Flt start_time, Flt dt)C {return _animation ? _animation->eventBetween  (from, to   , start_time, dt) : false;} // if           between events 'from' and 'to', 'start_time'=animation time at the start of the frame, 'dt'=animation time delta (should be set to "Time.d() * animation_speed")
   Flt  eventProgress(CChar8 *from, CChar8 *to, Flt       time        )C {return _animation ? _animation->eventProgress (from, to   ,       time    ) :     0;} // get progress between events 'from' and 'to',       'time'=animation time, 0 on fail

   Int  sbonToAbon(Int  sbon)C;                      // convert 'SkelBone' to 'AnimBone' index, -1        on fail
#if EE_PRIVATE
   Byte abonToSbon(Byte abon)C {return _bone[abon];} // convert 'AnimBone' to 'SkelBone' index, BONE_NULL on fail

   Bool load(C Str &name          ) {return false;} // this is unused, 'load' with 'user' is used instead
   Bool load(C Str &name, Ptr user);

   void zero();
#endif

            SkelAnim&   del(); // delete manually
           ~SkelAnim() {del();}
            SkelAnim();
   explicit SkelAnim(C Skeleton &skeleton, C Animation &animation);
            SkelAnim(C SkelAnim &src);
      void operator=(C SkelAnim &src);

private:
   Byte      *_bone; // #MeshVtxBoneHW
 C Animation *_animation;
};
/******************************************************************************/
#if EE_PRIVATE
void ShutAnimation();
#endif
/******************************************************************************/
