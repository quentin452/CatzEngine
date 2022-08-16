/******************************************************************************/
/******************************************************************************/
class ElmAnim : ElmData
{
   enum FLAG // !! these enums are saved !!
   {
      LOOP  =1<<0,
      LINEAR=1<<1,
      ROOT_DEL_POS_X=1<<2,
      ROOT_DEL_POS_Y=1<<3,
      ROOT_DEL_POS_Z=1<<4,
      ROOT_DEL_ROT_X=1<<5,
      ROOT_DEL_ROT_Y=1<<6,
      ROOT_DEL_ROT_Z=1<<7,
      ROOT_FROM_BODY=1<<8,
      ROOT_SMOOTH_ROT=1<<9,
      ROOT_SMOOTH_POS=1<<10,
      ROOT_DEL_ROT       =ROOT_DEL_ROT_X|ROOT_DEL_ROT_Y|ROOT_DEL_ROT_Z,
      ROOT_DEL_POS       =ROOT_DEL_POS_X|ROOT_DEL_POS_Y|ROOT_DEL_POS_Z,
      ROOT_SMOOTH_ROT_POS=ROOT_SMOOTH_ROT|ROOT_SMOOTH_POS,
      ROOT_ALL           =ROOT_DEL_POS|ROOT_DEL_ROT|ROOT_SMOOTH_ROT_POS|ROOT_FROM_BODY,
   };
   UID       skel_id;
   Pose      transform;
   Vec       root_move, root_rot;
   flt       fps;
   ushort    flag;
   Str       imported_file_params; // used to adjust anim events when reimporting with different params
   TimeStamp loop_time, linear_time, skel_time, file_time;

   // get
   bool loop  ()C;   ElmAnim& loop  (bool on);
   bool linear()C;   ElmAnim& linear(bool on);

   bool equal(C ElmAnim &src)C;
   bool newer(C ElmAnim &src)C;

   bool rootMove    (           )C; // use EqualMem to allow encoding zero as -0
   bool rootRot     (           )C; // use EqualMem to allow encoding zero as -0
   void rootMoveZero(           ); 
   void rootRotZero (           ); 
   void rootMove    (C Vec &move);  // encode as -0
   void rootRot     (C Vec &rot );  // encode as -0
   uint rootFlags   (           )C;
   void setRoot(Animation &anim);  

   virtual bool mayContain(C UID &id)C override;

   // operations
   virtual void newData()override;
   void from(C Animation &anim);  
   uint undo(C ElmAnim &src);
   uint sync(C ElmAnim &src);
   bool syncFile(C ElmAnim &src);

   // io
   static uint OldFlag1(ushort old);
   static uint OldFlag(byte old);
   void setImportedFileParams();
   void fix5();
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;

public:
   ElmAnim();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
