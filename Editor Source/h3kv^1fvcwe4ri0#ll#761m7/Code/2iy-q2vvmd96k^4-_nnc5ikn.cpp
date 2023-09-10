/******************************************************************************/
class AnimEditor : Viewport4Region
{
   static const bool always_draw_events=true;
   enum ANIM_OP
   {
      OP_ORN  ,
      OP_ORN2 ,
      OP_POS  ,
      OP_SCALE,
      OP_NUM  ,
   }
   enum TRACK_TYPE
   {
      TRACK_ORN,
      TRACK_POS,
      TRACK_SCALE,
      TRACK_NUM,
   }
   enum EVENT_OP
   {
      EVENT_MOVE,
      EVENT_RENAME,
      EVENT_NEW,
      EVENT_DEL,
   }
   static cchar8 *event_op_t[]=
   {
      "Move",
      "Rename",
      "Insert",
      "Delete",
   };
   static flt TimeEps()
   {
      flt w=AnimEdit.track.rect().w(); w=(w ? 1/w : 0);
      if(AnimEdit.anim)w*=AnimEdit.anim.length();
      return w*0.025;
   }

   class Change : Edit._Undo.Change
   {
      Animation anim;
      ElmAnim   anim_data;
      Skeleton  skel;

      virtual void create(ptr user)override
      {
         if(AnimEdit.anim)anim=*AnimEdit.anim;
         if(AnimEdit.skel)skel=*AnimEdit.skel;
         if(ElmAnim *anim_data=AnimEdit.data())T.anim_data=*anim_data;
         AnimEdit.undoVis();
      }
      virtual void apply(ptr user)override
      {
         if(AnimEdit.anim)if(ElmAnim *anim_data=AnimEdit.data())
         {
            CacheLock cl(Animations);
            Pose pose=anim_data.transform; // preserve pose, keep what we already have
            if(Elm *skel=Proj.findElm(T.anim_data.skel_id))if(ElmSkel *skel_data=skel.skelData())pose=skel_data.transform; // if the undo used a different skeleton, then we have to set animation pose to match it, because we will use it now, so always when the animation is linked to a skeleton then use its pose
           *AnimEdit.anim=anim;
            AnimEdit.anim->transform(GetTransform(T.anim_data.transform(), pose()), skel, false); // transform from undo matrix to new matrix
            anim_data.undo(T.anim_data); anim_data.undoSrcFile(T.anim_data); // call 'undoSrcFile' separately because it may not be called in 'undo', however we need it in case speed adjustment was performed
            anim_data.transform=pose;
            AnimEdit.setChanged(false);
            AnimEdit.toGui();
            AnimEdit.undoVis();
            AnimEdit.setMeshSkel();
            AnimEdit.setOrnTarget();
            Proj.refresh(false, false); // refresh any invalid skeleton link references
         }
      }
   }

   class Track : GuiCustom
   {
      bool events;
      int  event_lit=-1, event_sel=-1;

      void removedEvent(int index)
      {
         if(event_lit==index)event_lit=-1;else if(event_lit>index)event_lit--;
         if(event_sel==index)event_sel=-1;else if(event_sel>index)event_sel--;
      }
      flt screenToTime(flt x)C
      {
         if(C Animation *anim=AnimEdit.anim)
         {
            Rect r=screenRect();
            return Frac(LerpR(r.min.x, r.max.x, x))*anim.length();
         }
         return 0;
      }
      Track& create(bool events)
      {
         super.create();
         T.events=events;
         return T;
      }
      static flt ElmPosX(C Rect &rect, flt time)
      {
         return rect.lerpX(AnimEdit.timeToFrac(time));
      }
      static Rect ElmRect(C Rect &rect, flt time)
      {
         return Rect_C(rect.lerp(AnimEdit.timeToFrac(time), 0.5), 0.01, rect.h());
      }
      static flt Slide(flt &time, flt dt, flt length)
      {
         return time=Frac(time+dt, length);
      }
      virtual void update(C GuiPC &gpc)override
      {
         event_lit=-1;
         if(gpc.visible && visible() && Gui.ms()==this)if(Animation *anim=AnimEdit.anim)
         {
            Rect r=rect()+gpc.offset;
            flt  time=Frac(LerpR(r.min.x, r.max.x, Ms.pos().x))*anim.length();

            if(events)
            {
               // get lit
               flt dist=0;
               if((always_draw_events || AnimEdit.preview.event_op()>=0) && AnimEdit.preview.event_op()!=EVENT_NEW)REPA(anim.events)
               {
                  flt event_time=anim.events[i].time;
                  flt d=Abs(Ms.pos().x - ElmPosX(r, event_time));
                  if(event_lit<0 || d<dist)
                  {
                     event_lit=i; dist=d;
                     
                     // check how many events share the same time
                     int j=i; for(; j>0 && Equal(anim.events[j-1].time, event_time); )j--; // same is j..i inclusive
                     int same=i-j+1; if(same>1) // if more than 1
                     { // calculate based on Mouse Y
                        flt frac=LerpR(r.min.y, r.max.y, Ms.pos().y); // 0..1
                        event_lit=Mid(Trunc(frac*same)+j, j, i);
                     }
                  }
               }
               if(dist>0.05)event_lit=-1;

               if(AnimEdit.preview.event_op()==EVENT_RENAME && Ms.bp(0))RenameEvent.activate(event_lit);
               if(AnimEdit.preview.event_op()==EVENT_DEL    && Ms.bp(0))AnimEdit   .delEvent(event_lit);
               if(AnimEdit.preview.event_op()==EVENT_NEW    && Ms.bp(0))AnimEdit   .newEvent(time);
               if(AnimEdit.preview.event_op()==EVENT_MOVE   && Ms.b (0))
               {
                  if(Ms.bp(0))event_sel=event_lit; AnimEdit.moveEvent(event_sel, time);
                              event_lit=event_sel;
               }

               // set time
               if(AnimEdit.preview.event_op()<0 && Ms.b(0))AnimEdit.animTime(time);
            }else
            {
               // set time
               if(Ms.b(0))AnimEdit.animTime(time);

               // move KeyFrame
               if(Ms.b(1))if(AnimKeys *keys=AnimEdit.findKeys(AnimEdit.sel_bone))
               {
                  AnimKeys *keys_parent=AnimEdit.findKeysParent(AnimEdit.sel_bone, false); // skip sliding for root
                  AnimEdit.undos.set("slide");
                  flt dt=Ms.dc().x*anim.length()/rect().w();
                  if(Kb.ctrlCmd()) // all
                  {
                     if(Kb.alt() ? Ms.bp(1) : true) // for alt, slide only one time at the start
                     {
                        if(Kb.alt())dt=-AnimEdit.animTime();
                        if(AnimEdit.sel_bone<0) // entire animation
                        {
                           AnimEdit.anim.offsetTime(dt);
                        }else
                        {
                           REPA(keys.orns  )Slide(keys.orns  [i].time, dt, anim.length());
                           REPA(keys.poss  )Slide(keys.poss  [i].time, dt, anim.length());
                           REPA(keys.scales)Slide(keys.scales[i].time, dt, anim.length());
                           if(AnimEdit.op()==OP_ORN2 && keys_parent)
                           {
                              REPA(keys_parent.orns  )Slide(keys_parent.orns  [i].time, dt, anim.length());
                              REPA(keys_parent.poss  )Slide(keys_parent.poss  [i].time, dt, anim.length());
                              REPA(keys_parent.scales)Slide(keys_parent.scales[i].time, dt, anim.length());
                           }
                        }
                     }
                     AnimEdit.key_time=AnimEdit.animTime();
                  }else
                  {
                     if(Ms.bp(1))AnimEdit.key_time=AnimEdit.animTime();
                     switch(AnimEdit.op())
                     {
                        case OP_ORN  : if(AnimKeys.Orn   *key=FindOrn  (*keys, AnimEdit.key_time))AnimEdit.key_time=Slide(key.time, dt, anim.length());else AnimEdit.key_time=AnimEdit.animTime(); break;
                        case OP_POS  : if(AnimKeys.Pos   *key=FindPos  (*keys, AnimEdit.key_time))AnimEdit.key_time=Slide(key.time, dt, anim.length());else AnimEdit.key_time=AnimEdit.animTime(); break;
                        case OP_SCALE: if(AnimKeys.Scale *key=FindScale(*keys, AnimEdit.key_time))AnimEdit.key_time=Slide(key.time, dt, anim.length());else AnimEdit.key_time=AnimEdit.animTime(); break;
                        case OP_ORN2 :
                        {
                           if(AnimKeys.Orn *key=FindOrn(*keys, AnimEdit.key_time))
                           {
                              if(keys_parent)if(AnimKeys.Orn *key=FindOrn(*keys_parent, AnimEdit.key_time))Slide(key.time, dt, anim.length()); // !! first process the parent without modifying the 'key_time' !!
                                                                                        AnimEdit.key_time =Slide(key.time, dt, anim.length());
                           }else AnimEdit.key_time=AnimEdit.animTime();
                        }break;
                     }
                  }
                  keys.sortFrames().setTangents(anim.loop(), anim.length());
                  anim.setRootMatrix();
                  flt t=AnimEdit.animTime(); AnimEdit.animTime(Slide(t, dt, anim.length()));
                  AnimEdit.setChanged();
               }
            }
         }
      }
      static void DrawKey(flt time, C Rect &rect, flt y, flt r)
      {
         VI.dot(Vec2(ElmPosX(rect, time), y), r);
      }
      static void DrawKeys(AnimKeys &keys, C Rect &rect, C Color &color, flt y, flt r, int row, bool lit=false)
      {
         VI.color(color);
         switch(row)
         {
            case TRACK_ORN  : REPA(keys.orns  )DrawKey(keys.orns  [i].time, rect, y, r); break;
            case TRACK_POS  : REPA(keys.poss  )DrawKey(keys.poss  [i].time, rect, y, r); break;
            case TRACK_SCALE: REPA(keys.scales)DrawKey(keys.scales[i].time, rect, y, r); break;
         }
         VI.end();

         if(lit)switch(row)
         {
            case TRACK_ORN  : if(AnimEdit.op()==OP_ORN || AnimEdit.op()==OP_ORN2)if(AnimKeys.Orn   *key=FindOrn  (keys, AnimEdit.animTime())){VI.color(WHITE); DrawKey(key.time, rect, y, r); VI.end();} break;
            case TRACK_POS  : if(AnimEdit.op()==OP_POS                          )if(AnimKeys.Pos   *key=FindPos  (keys, AnimEdit.animTime())){VI.color(WHITE); DrawKey(key.time, rect, y, r); VI.end();} break;
            case TRACK_SCALE: if(AnimEdit.op()==OP_SCALE                        )if(AnimKeys.Scale *key=FindScale(keys, AnimEdit.animTime())){VI.color(WHITE); DrawKey(key.time, rect, y, r); VI.end();} break;
         }
      }
      virtual void draw(C GuiPC &gpc)override
      {
         if(gpc.visible && visible())
         {
            D.clip(gpc.clip);
            Rect r=rect()+gpc.offset;
            r.draw(GREY);
            if(C Animation *anim=AnimEdit.getVisAnim())
            {
               ElmRect(r, AnimEdit.animTime()).draw(BLACK); // draw time position
               if(events)
               {
                  TextStyleParams ts; ts.size=(AnimEdit.preview.event_op()>=0 ? 0.04 : 0.035); ts.align.set(0, 1); ts.color=ColorAlpha(0.6);
                  TextStyleParams ts_lit=ts; ts_lit.color=LitColor; ts_lit.size*=1.1;
                  flt  last_x=-FLT_MAX, y=r.max.y;
                  Vec2 pos; pos.y=y;
                  FREPA(anim.events) // draw events, from start
                  {
                   C AnimEvent &event=anim.events[i];
                     Rect e=ElmRect(r, event.time);
                     e.draw((event_lit==i) ? LitColor : LitSelColor); if((always_draw_events || AnimEdit.preview.event_op()>=0))
                     {
                        pos.x=e.centerX();
                        flt w=(ts.textWidth(event.name)+ts.size.y)/2, l=pos.x-w, r=pos.x+w;
                        if(l<=last_x)pos.y+=ts.size.y;else pos.y=y;
                        MAX(last_x, r);
                        D.text((event_lit==i) ? ts_lit : ts, pos, event.name);
                     }
                  }
               }else
               {
                  r.draw(Color(0, 64), false);

                  AnimKeys *sel_keys=AnimEdit.findVisKeys(AnimEdit.sel_bone),
                           *lit_keys=AnimEdit.findVisKeys(AnimEdit.lit_bone, false); // don't display root keys for highlighted
                  const flt s=r.h()/(TRACK_NUM+1)/2;
                  FREP(TRACK_NUM)
                  {
                     bool sel=false;
                     switch(AnimEdit.op())
                     {
                        case OP_ORN  :
                        case OP_ORN2 : sel=(i==TRACK_ORN  ); break;
                        case OP_POS  : sel=(i==TRACK_POS  ); break;
                        case OP_SCALE: sel=(i==TRACK_SCALE); break;
                     }
                     flt y=r.lerpY((TRACK_NUM-i)/flt(TRACK_NUM+1));
                     D.lineX(ColorAlpha(WHITE, sel ? 0.5 : 0.19), y, r.min.x, r.max.x);
                     Color color=ColorI(i);
                     if(lit_keys)DrawKeys(*lit_keys, r, ColorAlpha(Lerp(color, WHITE, 0.6), 0.5), y, s*1.1, i);
                     if(sel_keys)DrawKeys(*sel_keys, r,                 color                   , y, s*0.5, i, true);
                  }

                  D.clip();
                  bool   editing=false; if(Ms.b(1))if(Edit.Viewport4.View *view=AnimEdit.v4.getView(Gui.ms()))editing=true;
                  Vec2         p=AnimEdit.screenPos()+Vec2(0.01, -0.25);
                  int       bone=(editing ? AnimEdit.sel_bone : AnimEdit.lit_bone);
                  Skeleton *skel=AnimEdit.skel;
                  flt frame, frames; Str frame_t; if(AnimEdit.timeToFrame(AnimEdit.animTime(), frame) && AnimEdit.timeToFrame(anim.length(), frames))frame_t=S+", "+Round(frame)+'/'+Round(frames)+"f";
                  D.text(ObjEdit.ts, p, S+"Time: "+AnimEdit.animTime()+'/'+anim.length()+"s ("+Round(AnimEdit.timeToFrac(AnimEdit.animTime())*100)+'%'+frame_t+')'); p.y-=ObjEdit.ts.size.y;
                  if(1)
                  {      
                     D.text(ObjEdit.ts, p, S+"Speed: "+anim.rootSpeed()); p.y-=ObjEdit.ts.size.y;
                  }
                  if(skel && InRange(bone, skel.bones))
                  {
                     D.text(ObjEdit.ts, p, S+"Bone \""+skel.bones[bone].name+"\", Parent: "+(InRange(skel.bones[bone].parent, skel.bones) ? S+'"'+skel.bones[skel.bones[bone].parent].name+'"' : S+"none")); p.y-=ObjEdit.ts.size.y;
                     if(editing && AnimEdit.op()==OP_POS && InRange(bone, AnimEdit.anim_skel.bones))
                        D.text(ObjEdit.ts, p, S+"Offset: "+AnimEdit.anim_skel.bones[bone].pos);
                  }
                  if(Gui.ms()==this)
                  {
                     flt frac=Frac(LerpR(r.min.x, r.max.x, Ms.pos().x)), time=frac*anim.length();
                     if(AnimEdit.timeToFrame(time, frame))frame_t=S+", "+Round(frame)+"f";else frame_t.clear();
                     TextStyleParams ts; ts.align.set(0, 1); ts.size=0.052;
                     Str t=S+time+"s ("+Round(frac*100)+'%'+frame_t+')';
                     flt w_2=ts.textWidth(t)/2, x=Ms.pos().x; Clamp(x, r.min.x+w_2, r.max.x-w_2);
                     D.text(ts, x, r.max.y, t);
                  }
               }
            }
         }
      }
   }

   class Preview : PropWin
   {
      ViewportSkin viewport;
      Button       split, apply_speed, edit, locate;
      Tabs         event_op;
      Track        track;
      bool         draw_bones=false, draw_slots=false, draw_axis=false, draw_plane=false;
      int          lit_slot=-1, sel_slot=-1;
      flt          time_speed=1, prop_max_x=0,
                   cam_yaw=PI, cam_pitch=0, cam_zoom=1;
      Camera       cam;
      Property    *length=null, *event=null;

      static void Play      (  Preview &editor, C Str &t) {AnimEdit.play.set(TextBool(t));}
      static Str  Play      (C Preview &editor          ) {return AnimEdit.play();}
      static void Loop      (  Preview &editor, C Str &t) {AnimEdit.setLoop(TextBool(t));}
      static Str  Loop      (C Preview &editor          ) {if(ElmAnim *d=AnimEdit.data())return d.loop(); return false;}
      static void Linear    (  Preview &editor, C Str &t) {AnimEdit.setLinear(TextBool(t));}
      static Str  Linear    (C Preview &editor          ) {if(ElmAnim *d=AnimEdit.data())return d.linear(); return false;}
      static void Target    (  Preview &editor, C Str &t) {AnimEdit.setTarget(t);}
      static Str  Target    (C Preview &editor          ) {return Proj.elmFullName(Proj.animToObj(AnimEdit.elm));}
      static void Split     (  Preview &editor          ) {SplitAnim.activate(AnimEdit.elm_id);}
      static void ApplySpeed(  Preview &editor          ) {AnimEdit.applySpeed();}

      static void Render()
      {
         switch(Renderer())
         {
            case RM_PREPARE:
            {
               if(AnimEdit.mesh)AnimEdit.mesh->draw(AnimEdit.anim_skel);

               if(AnimEdit.preview.draw_slots)REPAO(AnimEdit.slot_meshes).draw(AnimEdit.anim_skel);

               LightDir(!(ActiveCam.matrix.z*2+ActiveCam.matrix.x-ActiveCam.matrix.y), 1-D.ambientColorL()).add(false);
            }break;
         }
      }
      void setCam()
      {
       C MeshPtr &mesh=AnimEdit.mesh;
         Box box(0); if(mesh){if(mesh->is())box=mesh->ext;else if(AnimEdit.skel)box=*AnimEdit.skel;}
         flt dist=Max(0.1, GetDist(box));
         D.viewFrom(dist*0.01).viewRange(dist*24);
         SetCam(cam, box, cam_yaw, cam_pitch, cam_zoom);
      }
      static void Draw(Viewport &viewport) {AnimEdit.preview.draw();}
             void draw()
      {
         AnimEdit.setAnimSkel();
         if(AnimEdit.skel)
         {
          C AnimatedSkeleton &anim_skel=AnimEdit.anim_skel;

            // remember settings
            CurrentEnvironment().set();
            AMBIENT_MODE ambient  =D.  ambientMode(); D.  ambientMode(AMBIENT_FLAT);
            DOF_MODE     dof      =D.      dofMode(); D.      dofMode(    DOF_NONE);
            bool         eye_adapt=D.eyeAdaptation(); D.eyeAdaptation(       false);
            bool         astros   =AstrosDraw       ; AstrosDraw     =false;
            bool         ocean    =Water.draw       ; Water.draw     =false;
            flt fov=D.viewFov(), from=D.viewFrom(), range=D.viewRange();
            Renderer.allow_temporal=false; // disable Temporal because previous bone matrixes are incorrect

            // render
            setCam();
            Renderer(Preview.Render);

            Renderer.setDepthForDebugDrawing();
            
            bool line_smooth=D.lineSmooth(true); // this can be very slow, so don't use it everywhere
            SetMatrix();
            if(draw_plane){bool line_smooth=D.lineSmooth(false); Plane(VecZero, Vec(0, 1, 0)).drawInfiniteByResolution(Color(255, 128), 192); D.lineSmooth(line_smooth);} // disable line smoothing because it can be very slow for lots of full-screen lines
            if(draw_axis ){D.depthLock(false); MatrixIdentity.draw(); D.depthUnlock();}
            if(draw_bones || draw_slots)
            {
               D.depthLock(false);
               anim_skel.draw(draw_bones ? ColorAlpha(CYAN, 0.75) : TRANSPARENT/*, draw_slots ? ORANGE : TRANSPARENT*/);
               if(draw_slots)FREPAO(anim_skel.slots).draw((sel_slot==i && lit_slot==i) ? LitSelColor : (sel_slot==i || lit_slot==i) ? SelColor : Color(255, 128, 0, 255), SkelSlotSize);
               D.depthUnlock();
            }
            D.lineSmooth(line_smooth);

            // restore settings
            Renderer.allow_temporal=true;
            D.viewFov(fov).viewFrom(from).viewRange(range);
            D.      dofMode(dof      );
            D.  ambientMode(ambient  );
            D.eyeAdaptation(eye_adapt);
            AstrosDraw     =astros;
            Water.draw     =ocean;
         }
      }

      Animation* anim()C {return AnimEdit.anim;}
      void create()
      {
         Property *play, *obj;
 length=&add(); // length
         add("Loop"         , MemberDesc(DATA_BOOL).setFunc(Loop  , Loop  )).desc("If animation is looped");
         add("Linear"       , MemberDesc(DATA_BOOL).setFunc(Linear, Linear)).desc("If KeyFrame interpolation should be Linear, if disabled then Cubic is used.\nLinear is recommended for animations with high framerate.\nCubic is recommended for animations with low framerate.\n\nLinear interpolation is calculated faster than Cubic.\nAnimations with Linear interpolation can be optimized for smaller size than Cubic.\nIf the animation has low framerate, then Cubic will give smoother results than Linear.");
    obj=&add("Target Object", MemberDesc(DATA_STR ).setFunc(Target, Target)).desc("Drag and drop an object here to set it").elmType(ELM_OBJ);
         add();
         add("Display:");
   play=&add("Play"         , MemberDesc(DATA_BOOL).setFunc(Play, Play)).desc(S+"Keyboard Shortcut: "+Kb.ctrlCmdName()+"+P, "+Kb.ctrlCmdName()+"+D, Space");
         add("Anim Speed"   , MEMBER(AnimEditor, anim_speed)).range(-100, 100).desc("Animation speed");
         add("Bones"        , MEMBER(AnimEditor, preview.draw_bones)).desc("Keyboard Shortcut: Alt+B");
         add("Slots"        , MEMBER(AnimEditor, preview.draw_slots));
         add("Axis"         , MEMBER(AnimEditor, preview.draw_axis )).desc("Keyboard Shortcut: Alt+A");
         add("Plane"        , MEMBER(AnimEditor, preview.draw_plane)).desc("Display ground plane\nKeyboard Shortcut: Alt+P");
  event=&add("Events:");
         autoData(&AnimEdit);
         flt  h=0.043;
         Rect r=super.create("Animation Editor", Vec2(0.02, -0.02), 0.036, h, 0.35); button[1].show(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
         obj.textline.resize(Vec2(h, 0));
         prop_max_x=r.max.x;
         T+=split.create(Rect_RU(r.max.x, r.max.y, 0.15, 0.055), "Split").func(Split, T).desc(S+"Split Animation\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+S");
         T+=apply_speed.create(Rect_R(r.max.x, play.text.rect().centerY(), 0.23, h), "Apply Speed").func(ApplySpeed, T).desc("Animation length/speed will be adjusted according to current \"Anim Speed\".");
         T+=edit.create(Rect_L(r.max.x/2, r.min.y, 0.17, 0.055), "Edit").func(Fullscreen, AnimEdit).focusable(false).desc(S+"Edit Animation KeyFrames\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+E");
         T+=locate.create(Rect_R(r.max.x/2-0.01, r.min.y, 0.17, 0.055), "Locate").func(Locate, AnimEdit).focusable(false).desc("Locate this element in the Project");
         T+=viewport.create(Draw); viewport.fov=PreviewFOV;
         T+=event_op.create(Rect_LU(0.02, 0, 0.45, 0.110), 0, event_op_t, Elms(event_op_t));
         event_op.tab(EVENT_MOVE  ).rect(Rect(event_op.rect().lerp(0  , 0.5), event_op.rect().lerp(0.5, 1  ))).desc("Move event\nKeyboard Shortcut: F1");
         event_op.tab(EVENT_RENAME).rect(Rect(event_op.rect().lerp(0.5, 0.5), event_op.rect().lerp(1  , 1  ))).desc("Rename event\nKeyboard Shortcut: F2");
         event_op.tab(EVENT_NEW   ).rect(Rect(event_op.rect().lerp(0  , 0  ), event_op.rect().lerp(0.5, 0.5))).desc("Create new event\nHold Ctrl to don't show Rename Window\nKeyboard Shortcut: F3\n\nAlternatively press Insert Key to create event at current time.");
         event_op.tab(EVENT_DEL   ).rect(Rect(event_op.rect().lerp(0.5, 0  ), event_op.rect().lerp(1  , 0.5))).desc("Delete event\nKeyboard Shortcut: F4\n\nAlternatively press Ctrl+Delete Key to delete event at Mouse cursor.");
         T+=track.create(true);
         rect(Rect_C(0, 0, Min(1.7, D.w()*2), Min(1.07, D.h()*2)));
      }
      void toGui()
      {
         super.toGui();
         if(length)length.display(S+"Length: "+(anim() ? S+TextReal(anim().length(), -2)+"s" : S));
      }

      virtual Preview& hide     (            )  override {if(!AnimEdit.fullscreen)AnimEdit.set(null); super.hide(); return T;}
      virtual Rect     sizeLimit(            )C override {Rect r=super.sizeLimit(); r.min.set(1.0, 0.9); return r;}
      virtual Preview& rect     (C Rect &rect)  override
      {
         super   .rect(rect);
         track   .rect(Rect(prop_max_x, -clientHeight(), clientWidth(), -clientHeight()+0.09).extend(-0.02));
         viewport.rect(Rect(prop_max_x, track.rect().max.y+0.01, clientWidth(), 0).extend(-0.02));
         event_op.move(Vec2(0, track.rect().min.y-event_op.rect().min.y));
         if(event)event.text.pos(event_op.rect().lu()+Vec2(0, 0.025));
         return T;
      }
      virtual void update(C GuiPC &gpc)override
      {
         lit_slot=-1;
         flt old_time=AnimEdit.animTime();
         if(gpc.visible && visible())
         {
            if(contains(Gui.kb()))
            {
               KbSc edit(KB_E, KBSC_CTRL_CMD), split(KB_S, KBSC_CTRL_CMD), play(KB_P, KBSC_CTRL_CMD), play2(KB_D, KBSC_CTRL_CMD), bones(KB_B, KBSC_ALT), axis(KB_A, KBSC_ALT), plane(KB_P, KBSC_ALT), prev(KB_PGUP, KBSC_CTRL|KBSC_REPEAT), next(KB_PGDN, KBSC_CTRL|KBSC_REPEAT);
               if(Kb.bp(KB_F1))event_op.toggle(0);else
               if(Kb.bp(KB_F2))event_op.toggle(1);else
               if(Kb.bp(KB_F3))event_op.toggle(2);else
               if(Kb.bp(KB_F4))event_op.toggle(3);
               if(edit .pd()){edit .eat(); AnimEdit.toggleFullscreen();}else
               if(split.pd()){split.eat(); Split(T);}else
               if(bones.pd()){bones.eat(); draw_bones^=true; toGui();}else
               if(axis .pd()){axis .eat(); draw_axis ^=true; toGui();}else
               if(plane.pd()){plane.eat(); draw_plane^=true; toGui();}else
               if(prev .pd()){prev .eat(); PrevAnim(AnimEdit);}else
               if(next .pd()){next .eat(); NextAnim(AnimEdit);}else
               if(play .pd() || play2.pd()){play.eat(); play2.eat(); AnimEdit.playToggle();}

               if(Gui.kb().type()!=GO_TEXTLINE && Gui.kb().type()!=GO_CHECKBOX)
               {
                  if(Kb.bp(KB_INS  )                ){Kb.eat(KB_INS  ); AnimEdit.newEvent(AnimEdit.animTime());}
                  if(Kb.bp(KB_DEL  ) && Kb.ctrlCmd()){Kb.eat(KB_DEL  ); AnimEdit.delEvent(track.event_lit);}
                  if(Kb.bp(KB_SPACE)                ){Kb.eat(KB_SPACE); AnimEdit.playToggle();}
                  if(Kb.bp(KB_HOME ) && Kb.ctrlCmd()){Kb.eat(KB_HOME ); Start(AnimEdit);}
                  if(Kb.bp(KB_END  ) && Kb.ctrlCmd()){Kb.eat(KB_END  ); End  (AnimEdit);}
               }
            }
            if(Gui.ms   ()==&viewport)if(Ms.b(0) || Ms.b(MS_BACK)){cam_yaw-=Ms.d().x; cam_pitch+=Ms.d().y; Ms.freeze();}
            if(Gui.wheel()==&viewport)Clamp(cam_zoom*=ScaleFactor(Ms.wheel()*-0.2), 0.1, 32);
            REPA(Touches)if(Touches[i].guiObj()==&viewport && Touches[i].on()){cam_yaw-=Touches[i].ad().x*2.0; cam_pitch+=Touches[i].ad().y*2.0;}

            AnimEdit.playUpdate(time_speed);
            
            if(draw_slots && Gui.dragging())
            {
               viewport.setDisplayView();
               setCam();
               flt dist=0.03;
               REPA(AnimEdit.anim_skel.slots)
               {
                C OrientM &slot=AnimEdit.anim_skel.slots[i];
                  flt d; if(Distance2D(Ms.pos(), Edge(slot.pos, slot.pos+slot.perp*SkelSlotSize), d, 0))
                     if(d<dist){dist=d; lit_slot=i;}
               }
            }
         }
         super.update(gpc); // process after adjusting 'animTime' so clicking on anim track will not be changed afterwards

         flt delta_t=Time.ad()*time_speed; // use default delta
         if(AnimEdit.anim) // if we have animation
            if(flt length=AnimEdit.anim.length()) // if it has time
               {delta_t=Frac(AnimEdit.animTime()-old_time, length); if(delta_t>=length*0.5)delta_t-=length;} // use delta based on anim time delta
      }
   }

   class OptimizeAnim : PropWin
   {
      bool      refresh_needed=true, preview=true;
      dbl       angle_eps=EPS_ANIM_ANGLE, pos_eps=EPS_ANIM_POS, scale_eps=EPS_ANIM_SCALE;
      Property *file_size=null, *optimized_size=null;
      Button    optimize;
      Animation anim;

      static void Changed(C Property &prop) {AnimEdit.optimize_anim.refresh();}
      static void Optimize(OptimizeAnim &oa)
      {
         if(AnimEdit.anim)
         {
            AnimEdit.undos.set("optimize");
            oa.optimizeDo(*AnimEdit.anim);
            AnimEdit.setChanged();
         }
         oa.hide();
      }

      void optimizeDo(Animation &anim)
      {
         anim.optimize(angle_eps, pos_eps, scale_eps);
      }
      Animation* getAnim()
      {
         Animation *src=AnimEdit.anim;
         if(refresh_needed)
         {
            refresh_needed=false;
            if(src)anim=*src;else anim.del();
            File f;
            if(     file_size){anim.save(f.writeMem());      file_size.display(S+     "File Size: "+SizeKB(f.size()));}
            optimizeDo(anim);
            if(optimized_size){anim.save(f.reset   ()); optimized_size.display(S+"Optimized Size: "+SizeKB(f.size()));}
         }
         return preview ? &anim : src;
      }
      void refresh() {refresh_needed=true;}
      OptimizeAnim& create()
      {
         add("Preview"           , MEMBER(OptimizeAnim,   preview));
         add("Angle Tolerance"   , MEMBER(OptimizeAnim, angle_eps)).changed(Changed).mouseEditSpeed(0.001 ).range(0, PI);
         add("Position Tolerance", MEMBER(OptimizeAnim,   pos_eps)).changed(Changed).mouseEditSpeed(0.0001).min(0);
         add("Scale Tolerance"   , MEMBER(OptimizeAnim, scale_eps)).changed(Changed).mouseEditSpeed(0.0001).min(0);
              file_size=&add();
         optimized_size=&add();
         Rect r=super.create("Reduce Keyframes", Vec2(0.02, -0.02), 0.040, 0.046); button[2].show();
         autoData(this);
         resize(Vec2(0, 0.1));
         T+=optimize.create(Rect_D(clientWidth()/2, -clientHeight()+0.03, 0.2, 0.06), "Optimize").func(Optimize, T);
         hide();
         return T;
      }
   }

   class ScalePosKeys : PropWin
   {
      bool      refresh_needed=true, preview=true;
      Vec       scale=1;
      flt       scale_xyz=1;
      Button    ok;
      Animation anim;

      static void Changed(C Property &prop) {AnimEdit.scale_pos_keys.refresh();}
      static void OK(ScalePosKeys &oa)
      {
         if(AnimEdit.anim)
         {
            AnimEdit.undos.set("scalePos");
            oa.scalePos(*AnimEdit.anim);
            AnimEdit.setChanged();
         }
         oa.hide();
      }

      void scalePos(Animation &anim)
      {
         if(AnimKeys *keys=AnimEdit.findKeys(&anim, AnimEdit.sel_bone))
         {
            REPA(keys.poss)
            {
               Vec &pos=keys.poss[i].pos;
               pos.x*=scale.x;
               pos.y*=scale.y;
               pos.z*=scale.z;
               pos  *=scale_xyz;
            }
            keys.setTangents(anim.loop(), anim.length());
         }
         anim.setRootMatrix();
      }
      Animation* getAnim()
      {
         Animation *src=AnimEdit.anim;
         if(refresh_needed)
         {
            refresh_needed=false;
            if(src)anim=*src;else anim.del();
            scalePos(anim);
         }
         return preview ? &anim : src;
      }
      void refresh() {refresh_needed=true;}
      ScalePosKeys& create()
      {
         add("Preview"  , MEMBER(ScalePosKeys, preview  ));
         add("Scale X"  , MEMBER(ScalePosKeys, scale.x  )).changed(Changed).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
         add("Scale Y"  , MEMBER(ScalePosKeys, scale.y  )).changed(Changed).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
         add("Scale Z"  , MEMBER(ScalePosKeys, scale.z  )).changed(Changed).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
         add("Scale XYZ", MEMBER(ScalePosKeys, scale_xyz)).changed(Changed).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
         Rect r=super.create("Scale Pos Keys", Vec2(0.02, -0.02), 0.040, 0.046); button[2].show();
         autoData(this);
         resize(Vec2(0, 0.1));
         T+=ok.create(Rect_D(clientWidth()/2, -clientHeight()+0.03, 0.2, 0.06), "OK").func(OK, T);
         hide();
         return T;
      }
   }

   class TimeRangeSpeed : PropWin
   {
      flt    start, end, speed;
      Button apply;
      Property *st, *e, *sp;

      static void Apply(TimeRangeSpeed &tsr)
      {
         if(AnimEdit.anim && tsr.speed)
         {
            AnimEdit.undos.set("trs");
            AnimEdit.anim.scaleTime(tsr.start, tsr.end, 1/tsr.speed);
            AnimEdit.setChanged();
         }
         tsr.hide();
      }
      TimeRangeSpeed& create()
      {
         st=&add("Start Time", MEMBER(TimeRangeSpeed, start));
         e =&add("End Time"  , MEMBER(TimeRangeSpeed, end  ));
         sp=&add("Speed"     , MEMBER(TimeRangeSpeed, speed));
         Rect r=super.create("Time Range Speed", Vec2(0.02, -0.02), 0.040, 0.046); button[2].show();
         autoData(this);
         resize(Vec2(0, 0.1));
         T+=apply.create(Rect_D(clientWidth()/2, -clientHeight()+0.03, 0.2, 0.06), "Apply").func(Apply, T);
         hide();
         return T;
      }
      void display()
      {
         if(AnimEdit.anim)
         {
            st.range(0, AnimEdit.anim.length()).set(AnimEdit.animTime());
            e .range(0, AnimEdit.anim.length()).set(AnimEdit.track.screenToTime(Ms.pos().x));
            sp.set(AnimEdit.anim_speed);
            activate();
         }
      }
   }

   static void Fullscreen(AnimEditor &editor) {editor.toggleFullscreen();}

   static void Render()
   {
      switch(Renderer())
      {
         case RM_PREPARE:
         {
            if(AnimEdit.draw_mesh() && AnimEdit.mesh)AnimEdit.mesh->draw(AnimEdit.anim_skel);

            LightDir(!(ActiveCam.matrix.z*2+ActiveCam.matrix.x-ActiveCam.matrix.y), 1-D.ambientColorL()).add(false);
         }break;
      }
   }
   static void Draw(Viewport &viewport) {if(Edit.Viewport4.View *view=AnimEdit.v4.getView(&viewport))AnimEdit.draw(*view);}
          void draw(Edit.Viewport4.View &view)
   {
      setAnimSkel();
      view.camera.set();
      D.dofFocus(ActiveCam.dist);
      CurrentEnvironment().set();
      bool astros=AstrosDraw; AstrosDraw=false;
      bool ocean =Water.draw; Water.draw=false;
      Renderer.wire=wire();
      Renderer.allow_temporal=false; // disable Temporal because previous bone matrixes are incorrect

      Renderer(AnimEditor.Render);

      Renderer.allow_temporal=true;
      Renderer.wire=false;
      AstrosDraw=astros;
      Water.draw=ocean;

      // helpers using depth buffer
      bool line_smooth=D.lineSmooth(true); // this can be very slow, so don't use it everywhere
      Renderer.setDepthForDebugDrawing();
      if(     axis ()){SetMatrix(); MatrixIdentity.draw();}
      if(show_grid ()){SetMatrix(); bool line_smooth=D.lineSmooth(false); int size=8; Plane(VecZero, Vec(0, 1, 0)).drawLocal(Color(255, 128), size, false, size*(3*2)); D.lineSmooth(line_smooth);} // disable line smoothing because it can be very slow for lots of full-screen lines
      if(draw_bones() || settings()==0) // always draw axis for root when settings are visible
      {
         SetMatrix();
         D.depthLock(false);
         int drawn_matrix_for_bone=INT_MIN;
         if( draw_bones())
         {
            int sel_bone2=((op()==OP_ORN2) ? boneParent(sel_bone) : -1);
            REPA(anim_skel.bones)
            {
               SkelBone bone=transformedBone(i);
               bool has_keys=false; if(AnimKeys *keys=findVisKeys(i))switch(op())
               {
                  case OP_ORN  :
                  case OP_ORN2 : has_keys=(keys.orns  .elms()>0); break;
                  case OP_POS  : has_keys=(keys.poss  .elms()>0); break;
                  case OP_SCALE: has_keys=(keys.scales.elms()>0); break;
               }
               Color col=(has_keys ? PURPLE : CYAN);
               bool  lit=(lit_bone==i), sel=(sel_bone==i || sel_bone2==i);
               if(lit && sel)col=LitSelColor;else 
               if(       sel)col=   SelColor;else
               if(lit       )col=      col  ;else 
                             col.a=140;
               bone.draw(col);
            }
          //if(skel && InRange(sel_bone, skel.bones)) don't check this so we can draw just for root too
            {
               if(op()==OP_ORN || op()==OP_ORN2 )orn_target.draw(RED, 0.005);
               if(op()==OP_POS || op()==OP_SCALE
              || (op()==OP_ORN || op()==OP_ORN2 || (anim && anim.keys.is()) || settings()==0) && sel_bone<0 // always draw axis for root
               )DrawMatrix(transformedBoneAxis(drawn_matrix_for_bone=sel_bone), bone_axis);
            }
         }
         if(drawn_matrix_for_bone!=-1 && settings()==0)DrawMatrix(transformedBoneAxis(-1), -1); // always draw axis for root when (not yet drawn and settings are visible)
         D.depthUnlock();
      }
      D.lineSmooth(line_smooth);
   }

   UID               elm_id=UIDZero, mesh_id=UIDZero, skel_id=UIDZero;
   Elm              *elm=null;
   bool              changed=false, fullscreen=false, copied_bone_pos_relative=false;
   flt               blend_range=-1; // amount of seconds to use blend when operating on multiple keyframes (using Ctrl), use -1 to disable TODO: make this configurable via UI
   Animation        *anim=null;
   Skeleton         *skel=null, skel_data;
   AnimatedSkeleton  anim_skel;
   SkelAnim          skel_anim;
   Mesh              mesh_data;
   MeshPtr           mesh;
   Memc<SlotMesh>    slot_meshes;
   Preview           preview;
   Track             track;
   Button            axis, draw_bones, draw_mesh, show_grid, undo, redo, locate, edit, force_play, start, end, prev_frame, next_frame,
                     loop, linear, root_from_body,
                     root_del_pos, root_del_pos_x, root_del_pos_y, root_del_pos_z,
                     root_del_rot, root_del_rot_x, root_del_rot_y, root_del_rot_z,
                     root_del_scale, root_smooth, root_smooth_rot, root_smooth_pos, root_set_move, root_set_rot, reload;
   CheckBox          play;
   Memx<Property>    props, root_props;
   Tabs              op, settings;
   ComboBox          cmd;
   Region            settings_region;
   Text            t_settings, t_settings_root;
   TextWhite         ts;
   flt               anim_speed=1, key_time=0;
   int               lit_bone=-1, sel_bone=-1, bone_axis=-1;
   Str8              sel_bone_name;
   Vec               orn_target=0, orn_perp=0, copied_bone_pos=0;
   OptimizeAnim      optimize_anim;
   ScalePosKeys      scale_pos_keys;
   TimeRangeSpeed    time_range_speed;
   Edit.Undo<Change> undos(true);   void undoVis() {SetUndo(undos, undo, redo);}

   static cchar8 *transform_obj_dialog_id="animateObj";

   static void PrevAnim       (AnimEditor &editor) {Proj.elmNext(editor.elm_id, -1);}
   static void NextAnim       (AnimEditor &editor) {Proj.elmNext(editor.elm_id);}
   static void Mode1          (AnimEditor &editor) {editor.op.toggle(0);}
   static void Mode2          (AnimEditor &editor) {editor.op.toggle(1);}
   static void Mode3          (AnimEditor &editor) {editor.op.toggle(2);}
   static void Mode4          (AnimEditor &editor) {editor.op.toggle(3);}
   static void Play           (AnimEditor &editor) {editor.playToggle();}
   static void Identity       (AnimEditor &editor) {editor.axis    .push();}
   static void Settings       (AnimEditor &editor) {editor.settings.toggle(0);}
   static void Start          (AnimEditor &editor) {               editor.animTime(0);}
   static void End            (AnimEditor &editor) {if(editor.anim)editor.animTime(editor.anim.length());}
   static void PrevFrame      (AnimEditor &editor) {editor.frame(-1);}
   static void NextFrame      (AnimEditor &editor) {editor.frame(+1);}
   static void  DelFrame      (AnimEditor &editor) {editor.delFrame ();}
   static void  DelFrames     (AnimEditor &editor) {editor.delFrames(editor.sel_bone);}
   static void  DelFramesAtEnd(AnimEditor &editor) {editor.delFramesAtEnd();}
   static void FreezeDelFrame (AnimEditor &editor) {editor.freezeDelFrame();}
   static void FreezeDelFrames(AnimEditor &editor) {editor.freezeDelFrames();}
   static void Optimize       (AnimEditor &editor) {editor.optimize_anim.activate();}
   static void ScalePosKey    (AnimEditor &editor) {editor.scale_pos_keys.activate();}
   static void TimeRangeSp    (AnimEditor &editor) {editor.time_range_speed.display();}
   static void ReverseFrames  (AnimEditor &editor) {editor.reverseFrames();}
   static void ApplySpeed     (AnimEditor &editor) {editor.applySpeed();}
   static void FreezeBone     (AnimEditor &editor) {editor.freezeBone();}
   static void Mirror         (AnimEditor &editor) {if(editor.anim){editor.undos.set("mirror", true); Skeleton temp; editor.anim.mirror(editor.skel ? *editor.skel : temp); editor.setAnimSkel(); editor.setOrnTarget(); editor.setChanged(); editor.toGui();}}
   void rotate(C Matrix3 &m)
   {
      if(anim)
      {
         undos.set("rot", true);
         if(ElmAnim *d=data())if(d.rootMove()){Vec root_move=d.root_move*m; d.rootMove(root_move);} // !! don't do "d.root_move*=m" because we need to call 'rootMove' !!
         Skeleton temp; anim.transform(m, skel ? *skel : temp, true); setAnimSkel(); setOrnTarget(); setChanged(); toGui();
      }
   }
   static void RotX           (AnimEditor &editor) {editor.rotate(Matrix3().setRotateX(PI_2));}
   static void RotY           (AnimEditor &editor) {editor.rotate(Matrix3().setRotateY(PI_2));}
   static void RotZ           (AnimEditor &editor) {editor.rotate(Matrix3().setRotateZ(PI_2));}
   static void RotXH          (AnimEditor &editor) {editor.rotate(Matrix3().setRotateX(PI_4));}
   static void RotYH          (AnimEditor &editor) {editor.rotate(Matrix3().setRotateY(PI_4));}
   static void RotZH          (AnimEditor &editor) {editor.rotate(Matrix3().setRotateZ(PI_4));}
   static void DrawBones      (AnimEditor &editor) {editor.draw_bones.push();}
   static void DrawMesh       (AnimEditor &editor) {editor.draw_mesh .push();}
   static void Grid           (AnimEditor &editor) {editor.show_grid .push();}
   static void TransformObj   (AnimEditor &editor)
   {
      Dialog &dialog=Gui.getMsgBox(transform_obj_dialog_id);
      dialog.set("Transform Object", "Warning: this option will open the original object in the Object Editor, clear undo levels (if any) and transform it, including its Mesh and Skeleton according to current Animation. This cannot be undone. Are you sure you want to do this?", Memt<Str>().add("Yes").add("Yes (preserve this Animation)").add("Yes (preserve all Animations)").add("Cancel"));
      dialog.buttons[0].func(TransformObjYes            , editor).desc("This Animation will be transformed among other Object Animations to the new Skeleton.\nChoose this option if you intend to use this Animation on the transformed Object.");
      dialog.buttons[1].func(TransformObjYesPreserveThis, editor).desc("Other Object Animations will be transformed to the new Skeleton, except this one which will remain unmodified.\nChoose this option if you don't intend to use this Animation on the transformed Object, but instead keep it as backup to transform the original Object again in the future.");
      dialog.buttons[2].func(TransformObjYesPreserveAll , editor).desc("No Animations will be transformed to the new Skeleton, all will remain unmodified.\nChoose this option if you want to keep animations as they are.");
      dialog.buttons[3].func(Hide                       , SCAST(GuiObj, dialog));
      dialog.activate();
   }
   static void TransformObjYes            (AnimEditor &editor) {editor.transformObj();}
   static void TransformObjYesPreserveThis(AnimEditor &editor) {editor.transformObj(true, editor.elm_id);}
   static void TransformObjYesPreserveAll (AnimEditor &editor) {editor.transformObj(false);}
          void transformObj(bool transform_anims=true, C UID &ignore_anim_id=UIDZero)
   {
      Gui.closeMsgBox(transform_obj_dialog_id);
      if(anim)
      if(Elm *obj=Proj.animToObjElm(elm))
      {
         ObjEdit.activate(obj);
         ObjEdit.animate(*anim, animTime(), transform_anims, ignore_anim_id); // can't use 'anim_skel' because this may operate on game mesh/skel, which can be different if using "object body", we need mesh original skel
      }else Gui.msgBox(S, "There's no Object associated with this Animation.");
   }

   static void Undo  (AnimEditor &editor) {editor.undos.undo();}
   static void Redo  (AnimEditor &editor) {editor.undos.redo();}
   static void Locate(AnimEditor &editor) {Proj.elmLocate(editor.elm_id);}
   static void Reload(AnimEditor &editor) {Proj.elmReload(editor.elm_id);}

   static void Loop  (AnimEditor &editor) {editor.setLoop  (editor.loop  ());}
   static void Linear(AnimEditor &editor) {editor.setLinear(editor.linear());}

   static void RootDelPos(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())
      {
         editor.undos.set("rootDelPos");
         bool on=FlagOff(d.flag, ElmAnim.ROOT_DEL_POS);
         editor.root_del_pos_x.set(on, QUIET);
         editor.root_del_pos_y.set(on, QUIET);
         editor.root_del_pos_z.set(on, QUIET);
         FlagSet(d.flag, ElmAnim.ROOT_DEL_POS, on); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(on){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim.adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_POSITION); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui();} editor.setChanged();
      }
   }
   static void RootDelRot(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())
      {
         editor.undos.set("rootDelRot");
         bool on=FlagOff(d.flag, ElmAnim.ROOT_DEL_ROT);
         editor.root_del_rot_x.set(on, QUIET);
         editor.root_del_rot_y.set(on, QUIET);
         editor.root_del_rot_z.set(on, QUIET);
         FlagSet(d.flag, ElmAnim.ROOT_DEL_ROT, on); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(on){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim.adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_ROTATION); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui();} editor.setChanged();
      }
   }
   static void RootDel(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())if(editor.skel)
      {
         editor.undos.set("rootDel");
         uint flag=d.flag;
         FlagEnable(d.flag, ElmAnim.ROOT_DEL_POS|ElmAnim.ROOT_DEL_ROT);
         if(d.flag!=flag)
         {
            editor.anim.adjustForSameTransformWithDifferentSkeleton(*editor.skel, *editor.skel, -1, null, ROOT_DEL_POSITION|ROOT_DEL_ROTATION); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui(); editor.setChanged();
         }
      }
   }
   static void RootSmooth(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())
      {
         editor.undos.set("rootSmooth");
         bool on=FlagOff(d.flag, ElmAnim.ROOT_SMOOTH_ROT_POS);
         editor.root_smooth_rot.set(on, QUIET);
         editor.root_smooth_pos.set(on, QUIET);
         FlagSet(d.flag, ElmAnim.ROOT_SMOOTH_ROT_POS, on); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(on){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim.adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_SMOOTH_ROT_POS); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui();} editor.setChanged();
      }
   }
   static void RootDelPosX  (AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootDelPos"  ); FlagToggle(d.flag, ElmAnim.ROOT_DEL_POS_X ); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d.flag&ElmAnim.ROOT_DEL_POS_X ){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim.adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_POSITION_X); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   static void RootDelPosY  (AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootDelPos"  ); FlagToggle(d.flag, ElmAnim.ROOT_DEL_POS_Y ); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d.flag&ElmAnim.ROOT_DEL_POS_Y ){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim.adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_POSITION_Y); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   static void RootDelPosZ  (AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootDelPos"  ); FlagToggle(d.flag, ElmAnim.ROOT_DEL_POS_Z ); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d.flag&ElmAnim.ROOT_DEL_POS_Z ){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim.adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_POSITION_Z); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   static void RootDelRotX  (AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootDelRot"  ); FlagToggle(d.flag, ElmAnim.ROOT_DEL_ROT_X ); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d.flag&ElmAnim.ROOT_DEL_ROT_X ){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim.adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_ROTATION_X); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   static void RootDelRotY  (AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootDelRot"  ); FlagToggle(d.flag, ElmAnim.ROOT_DEL_ROT_Y ); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d.flag&ElmAnim.ROOT_DEL_ROT_Y ){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim.adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_ROTATION_Y); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   static void RootDelRotZ  (AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootDelRot"  ); FlagToggle(d.flag, ElmAnim.ROOT_DEL_ROT_Z ); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d.flag&ElmAnim.ROOT_DEL_ROT_Z ){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim.adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_ROTATION_Z); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
 //static void RootDelScale (AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootDelScale"); FlagToggle(d.flag, ElmAnim.ROOT_DEL_SCALE ); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d.flag&ElmAnim.ROOT_DEL_SCALE ){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim.adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_SCALE     ); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   static void RootSmoothRot(AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootSmooth"  ); FlagToggle(d.flag, ElmAnim.ROOT_SMOOTH_ROT); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d.flag&ElmAnim.ROOT_SMOOTH_ROT){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim.adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_SMOOTH_ROT    ); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   static void RootSmoothPos(AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootSmooth"  ); FlagToggle(d.flag, ElmAnim.ROOT_SMOOTH_POS); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d.flag&ElmAnim.ROOT_SMOOTH_POS){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim.adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_SMOOTH_POS    ); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   static void RootFromBody (AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())
      {
         editor.undos.set("rootFromBody"); FlagToggle(d.flag, ElmAnim.ROOT_FROM_BODY); /*d.file_time.getUTC(); already changed in 'setChanged' */
         if(d.flag&ElmAnim.ROOT_FROM_BODY)if(editor.skel)
         {
            editor.anim.adjustForSameTransformWithDifferentSkeleton(*editor.skel, *editor.skel, Max(0, editor.skel.findBoneI(BONE_SPINE)), null, d.rootFlags()); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui();
         }
         editor.setChanged();
      }
   }
   static void RootFromBodyX(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())if(editor.skel)
      {
         editor.undos.set("rootFromBody");
         uint flag=d.flag;
         FlagDisable(d.flag, ElmAnim.ROOT_DEL_POS_X);
         FlagEnable (d.flag, ElmAnim.ROOT_DEL_POS_Y|ElmAnim.ROOT_DEL_POS_Z|ElmAnim.ROOT_DEL_ROT|ElmAnim.ROOT_FROM_BODY);
         if(d.flag!=flag)
         {
            editor.anim.adjustForSameTransformWithDifferentSkeleton(*editor.skel, *editor.skel, Max(0, editor.skel.findBoneI(BONE_SPINE)), null, d.rootFlags()); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui(); editor.setChanged();
         }
      }
   }
   static void RootFromBodyZ(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())if(editor.skel)
      {
         editor.undos.set("rootFromBody");
         uint flag=d.flag;
         FlagDisable(d.flag, ElmAnim.ROOT_DEL_POS_Z);
         FlagEnable (d.flag, ElmAnim.ROOT_DEL_POS_X|ElmAnim.ROOT_DEL_POS_Y|ElmAnim.ROOT_DEL_ROT|ElmAnim.ROOT_FROM_BODY);
         if(d.flag!=flag)
         {
            editor.anim.adjustForSameTransformWithDifferentSkeleton(*editor.skel, *editor.skel, Max(0, editor.skel.findBoneI(BONE_SPINE)), null, d.rootFlags()); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui(); editor.setChanged();
         }
      }
   }
   static void RootFromBodyXZ(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())if(editor.skel)
      {
         editor.undos.set("rootFromBody");
         uint flag=d.flag;
         FlagDisable(d.flag, ElmAnim.ROOT_DEL_POS_X|ElmAnim.ROOT_DEL_POS_Z|ElmAnim.ROOT_DEL_ROT_Y);
         FlagEnable (d.flag, ElmAnim.ROOT_DEL_POS_Y|ElmAnim.ROOT_DEL_ROT_X|ElmAnim.ROOT_DEL_ROT_Z|ElmAnim.ROOT_FROM_BODY);
         if(d.flag!=flag)
         {
            editor.anim.adjustForSameTransformWithDifferentSkeleton(*editor.skel, *editor.skel, Max(0, editor.skel.findBoneI(BONE_SPINE)), null, d.rootFlags()); editor.setAnimSkel(); editor.setOrnTarget(); editor.toGui(); editor.setChanged();
         }
      }
   }
   static void RootSetMove(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())
      {
         editor.undos.set("rootMove");
         if(editor.root_set_move())
         {
            d.rootMove(editor.anim.rootTransform().pos/d.transform.scale);
            d.setRoot(*editor.anim);
            editor.setAnimSkel();
            editor.setOrnTarget();
         }else d.rootMoveZero();
       /*d.file_time.getUTC(); already changed in 'setChanged' */
         editor.setChanged();
      }
   }
   static void RootSetRot(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())
      {
         editor.undos.set("rootRot");
         if(editor.root_set_rot())
         {
            d.rootRot(editor.anim.rootTransform().axisAngle());
            d.setRoot(*editor.anim);
            editor.setAnimSkel();
            editor.setOrnTarget();
         }else d.rootRotZero();
       /*d.file_time.getUTC(); already changed in 'setChanged' */
         editor.setChanged();
      }
   }
   static Str  RootMoveX(C AnimEditor &editor) {if(ElmAnim *d=editor.data())if(d.rootMove())return d.root_move.x*d.transform.scale; if(editor.anim)return editor.anim.rootTransform().pos.x; return S;}
   static Str  RootMoveY(C AnimEditor &editor) {if(ElmAnim *d=editor.data())if(d.rootMove())return d.root_move.y*d.transform.scale; if(editor.anim)return editor.anim.rootTransform().pos.y; return S;}
   static Str  RootMoveZ(C AnimEditor &editor) {if(ElmAnim *d=editor.data())if(d.rootMove())return d.root_move.z*d.transform.scale; if(editor.anim)return editor.anim.rootTransform().pos.z; return S;}
   static void RootMoveX(  AnimEditor &editor, C Str &t) {if(ElmAnim *d=editor.data()){editor.undos.set("rootMove"); editor.root_set_move.set(true, QUIET); if(!d.rootMove())d.root_move=editor.anim.rootTransform().pos/d.transform.scale; d.root_move.x=TextFlt(t)/d.transform.scale; d.rootMove(d.root_move); /*d.file_time.getUTC(); already changed in 'setChanged' */ d.setRoot(*editor.anim); editor.setAnimSkel(); editor.setOrnTarget(); editor.setChanged();}}
   static void RootMoveY(  AnimEditor &editor, C Str &t) {if(ElmAnim *d=editor.data()){editor.undos.set("rootMove"); editor.root_set_move.set(true, QUIET); if(!d.rootMove())d.root_move=editor.anim.rootTransform().pos/d.transform.scale; d.root_move.y=TextFlt(t)/d.transform.scale; d.rootMove(d.root_move); /*d.file_time.getUTC(); already changed in 'setChanged' */ d.setRoot(*editor.anim); editor.setAnimSkel(); editor.setOrnTarget(); editor.setChanged();}}
   static void RootMoveZ(  AnimEditor &editor, C Str &t) {if(ElmAnim *d=editor.data()){editor.undos.set("rootMove"); editor.root_set_move.set(true, QUIET); if(!d.rootMove())d.root_move=editor.anim.rootTransform().pos/d.transform.scale; d.root_move.z=TextFlt(t)/d.transform.scale; d.rootMove(d.root_move); /*d.file_time.getUTC(); already changed in 'setChanged' */ d.setRoot(*editor.anim); editor.setAnimSkel(); editor.setOrnTarget(); editor.setChanged();}}

   static Str  RootRotX(C AnimEditor &editor) {if(ElmAnim *d=editor.data())if(d.rootRot())return TextReal(RadToDeg(d.root_rot.x), 1); if(editor.anim)return TextReal(RadToDeg(editor.anim.rootTransform().axisAngle().x), 1); return S;}
   static Str  RootRotY(C AnimEditor &editor) {if(ElmAnim *d=editor.data())if(d.rootRot())return TextReal(RadToDeg(d.root_rot.y), 1); if(editor.anim)return TextReal(RadToDeg(editor.anim.rootTransform().axisAngle().y), 1); return S;}
   static Str  RootRotZ(C AnimEditor &editor) {if(ElmAnim *d=editor.data())if(d.rootRot())return TextReal(RadToDeg(d.root_rot.z), 1); if(editor.anim)return TextReal(RadToDeg(editor.anim.rootTransform().axisAngle().z), 1); return S;}
   static void RootRotX(  AnimEditor &editor, C Str &t) {if(ElmAnim *d=editor.data()){editor.undos.set("rootRot"); editor.root_set_rot.set(true, QUIET); if(!d.rootRot())d.root_rot=editor.anim.rootTransform().axisAngle(); d.root_rot.x=DegToRad(TextFlt(t)); d.rootRot(d.root_rot); /*d.file_time.getUTC(); already changed in 'setChanged' */ d.setRoot(*editor.anim); editor.setAnimSkel(); editor.setOrnTarget(); editor.setChanged();}}
   static void RootRotY(  AnimEditor &editor, C Str &t) {if(ElmAnim *d=editor.data()){editor.undos.set("rootRot"); editor.root_set_rot.set(true, QUIET); if(!d.rootRot())d.root_rot=editor.anim.rootTransform().axisAngle(); d.root_rot.y=DegToRad(TextFlt(t)); d.rootRot(d.root_rot); /*d.file_time.getUTC(); already changed in 'setChanged' */ d.setRoot(*editor.anim); editor.setAnimSkel(); editor.setOrnTarget(); editor.setChanged();}}
   static void RootRotZ(  AnimEditor &editor, C Str &t) {if(ElmAnim *d=editor.data()){editor.undos.set("rootRot"); editor.root_set_rot.set(true, QUIET); if(!d.rootRot())d.root_rot=editor.anim.rootTransform().axisAngle(); d.root_rot.z=DegToRad(TextFlt(t)); d.rootRot(d.root_rot); /*d.file_time.getUTC(); already changed in 'setChanged' */ d.setRoot(*editor.anim); editor.setAnimSkel(); editor.setOrnTarget(); editor.setChanged();}}

   static void SetSelMirror(AnimEditor &editor) {editor.setSelMirror(false);}
   static void SetMirrorSel(AnimEditor &editor) {editor.setSelMirror(true );}
          void setSelMirror(int i, int bone_i, bool set_other)
   {
      if(set_other)Swap(i, bone_i);
      AnimKeys *src=findKeys(i, false), *dest=(src ? getKeys(bone_i, false) : findKeys(bone_i, false));
      if(dest)
      {
         if(src)
         {
           *dest=*src;
            dest.mirrorX();
         }else anim.bones.remove(bone_i, true);
      }
      setAnimSkel();
      setOrnTarget();
      setChanged();
   }
   void setSelMirror(bool set_other)
   {
      if(skel)
      {
         undos.set("boneMirror", true);
         int bone_i=(sel_bone>=0 ? sel_bone : lit_bone);
         if(SkelBone *bone=skel.bones.addr(bone_i))
         {
            Str bone_name=BoneNeutralName(bone.name);
            REPA(skel.bones)if(i!=bone_i && bone_name==BoneNeutralName(skel.bones[i].name))
            {
               setSelMirror(i, bone_i, set_other);
               return;
            }
            int mirror_type_index=-bone.type_index-1;
            int i=skel.findBoneI(bone.type, mirror_type_index, bone.type_sub);
            if(i>=0)setSelMirror(i, bone_i, set_other);
         }
      }
   }
   static void SkelBonePosCopy (AnimEditor &editor) {editor.skelBonePosCopy(false);}
   static void SkelBonePosCopyR(AnimEditor &editor) {editor.skelBonePosCopy(true );}
          void skelBonePosCopy (bool relative)
   {
      setAnimSkel();
    //copied_bone_pos=anim_skel.boneRoot(sel_bone).pos;
      copied_bone_pos=   transformedBone(sel_bone).pos;
      if(relative && sel_bone>=0)copied_bone_pos/=(Matrix)transformedBone(-1); // set as relative to root
      copied_bone_pos_relative=relative;
   }
   static void SkelBonePosPaste (AnimEditor &editor) {editor.skelBonePosPaste(Vec(1, 1, 1));}
   static void SkelBonePosPasteX(AnimEditor &editor) {editor.skelBonePosPaste(Vec(1, 0, 0));}
   static void SkelBonePosPasteY(AnimEditor &editor) {editor.skelBonePosPaste(Vec(0, 1, 0));}
   static void SkelBonePosPasteZ(AnimEditor &editor) {editor.skelBonePosPaste(Vec(0, 0, 1));}
          void skelBonePosPaste (C Vec &mask)
   {
      if(skel && anim)
      {
         undos.set("bonePosPaste");
         setAnimSkel();
       //Vec pos=anim_skel.boneRoot(sel_bone).pos;
         Vec pos=   transformedBone(sel_bone).pos;
         if(copied_bone_pos_relative && sel_bone>=0)pos/=(Matrix)transformedBone(-1);
         Vec delta=(copied_bone_pos-pos)*mask; if(delta.any())
         {
            anim.offsetRootBones(*skel, delta);
            setAnimSkel();
            setOrnTarget();
            setChanged();
            toGui();
         }
      }
   }

   bool selected()C {return Mode()==MODE_ANIM;}
   void selectedChanged()
   {
      setMenu();
      flush();
   }

   ElmAnim* data()C {return elm ? elm.animData() : null;}
   Animation* getVisAnim() {return scale_pos_keys.visibleOnActiveDesktop() ? scale_pos_keys.getAnim() : optimize_anim.visibleOnActiveDesktop() ? optimize_anim.getAnim() : anim;}

   flt  timeToFrac (flt time)C {return (anim && anim.length()) ? time/anim.length() : 0;}
   bool timeToFrame(flt time, flt &frame)C
   {
      if(ElmAnim *d=data())if(d.fps>0)
      {
         frame=time*d.fps;
         FileParams fps=d.imported_file_params; if(C TextParam *p=fps.findParam("speed"))frame*=Abs(p.asFlt());
         return true;
      }
      frame=0; return false;
   }
   flt  animTime(        )C {return _anim_time;}
   void animTime(flt time)
   {
      if(_anim_time!=time)
      {
        _anim_time=time;
         setAnimSkel();
         setOrnTarget();
      }
   }
   void setLoop(bool loop)
   {
      if(ElmAnim *d=data())if(d.loop()!=loop)
      {
         undos.set("loop"); d.loop(loop); d.loop_time.getUTC(); if(anim)anim.loop(loop); toGui(); setAnimSkel(); setOrnTarget(); setChanged(false); // set 'file' to false because the Editor will recreate the animation file upon receiving new data from server
      }
   }
   void setLinear(bool linear)
   {
      if(ElmAnim *d=data())if(d.linear()!=linear)
      {
         undos.set("linear"); d.linear(linear); d.linear_time.getUTC(); if(anim)anim.linear(linear); toGui(); setAnimSkel(); setOrnTarget(); setChanged(false); // set 'file' to false because the Editor will recreate the animation file upon receiving new data from server
      }
   }
   void selBone(int bone)
   {
      sel_bone=bone;
      setOrnTarget();
   }
   void setOrnTarget()
   {
      SkelBone       bone       =transformedBone(           sel_bone ), bone_parent;
      if(sel_bone>=0)bone_parent=transformedBone(boneParent(sel_bone)); // set 'bone_parent' only if we have a bone selected, otherwise (when root is selected) then keep it as identity

      orn_target=bone.to();

      orn_perp=bone.pos-NearestPointOnLine(bone.pos, bone_parent.pos, !(orn_target-bone_parent.pos));
      if(!orn_perp.normalize())orn_perp.set(0, 0, 1);
   }
   void setMenu()
   {
      super.setMenu(selected());
      cmd.menu.enabled(selected());
   }
   void setMenu(Node<MenuElm> &menu, C Str &prefix)
   {
      super.setMenu(menu, prefix);
      FREPA(menu.children)if(menu.children[i].name==prefix+"View")
      {
         Node<MenuElm> &v=menu.children[i];
         v.New().create("Mode 1"     , Mode1     , T, true).kbsc(KbSc(KB_F1));
         v.New().create("Mode 2"     , Mode2     , T, true).kbsc(KbSc(KB_F2));
         v.New().create("Mode 3"     , Mode3     , T, true).kbsc(KbSc(KB_F3));
         v.New().create("Mode 4"     , Mode4     , T, true).kbsc(KbSc(KB_F4));
         v.New().create("Edit"       , Fullscreen, T).kbsc(KbSc(KB_E    , KBSC_CTRL_CMD)).flag(MENU_HIDDEN);
         v.New().create("Play"       , Play      , T).kbsc(KbSc(KB_P    , KBSC_CTRL_CMD)).kbsc1(KbSc(KB_D, KBSC_CTRL_CMD)).flag(MENU_HIDDEN);
         v.New().create("Bones"      , DrawBones , T).kbsc(KbSc(KB_B    , KBSC_ALT     )).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Mesh"       , DrawMesh  , T).kbsc(KbSc(KB_M    , KBSC_ALT     )).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Grid"       , Grid      , T).kbsc(KbSc(KB_G    , KBSC_ALT     )).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Axis"       , Identity  , T).kbsc(KbSc(KB_A    , KBSC_ALT     )).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Settings"   , Settings  , T).kbsc(KbSc(KB_S    , KBSC_ALT     )).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Start"      , Start     , T).kbsc(KbSc(KB_HOME , KBSC_CTRL_CMD)).kbsc1(KbSc(KB_LBR  , KBSC_CTRL_CMD)).flag(MENU_HIDDEN);
         v.New().create("End"        , End       , T).kbsc(KbSc(KB_END  , KBSC_CTRL_CMD)).kbsc1(KbSc(KB_RBR  , KBSC_CTRL_CMD)).flag(MENU_HIDDEN);
         v.New().create("Prev Frame" , PrevFrame , T).kbsc(KbSc(KB_LEFT , KBSC_CTRL_CMD|KBSC_REPEAT)).kbsc1(KbSc(KB_COMMA, KBSC_CTRL_CMD|KBSC_REPEAT)).flag(MENU_HIDDEN);
         v.New().create("Next Frame" , NextFrame , T).kbsc(KbSc(KB_RIGHT, KBSC_CTRL_CMD|KBSC_REPEAT)).kbsc1(KbSc(KB_DOT  , KBSC_CTRL_CMD|KBSC_REPEAT)).flag(MENU_HIDDEN);
         v.New().create("Prev Frames", PrevFrame , T).kbsc(KbSc(KB_LEFT , KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_REPEAT)).kbsc1(KbSc(KB_COMMA, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_REPEAT)).flag(MENU_HIDDEN);
         v.New().create("Next Frames", NextFrame , T).kbsc(KbSc(KB_RIGHT, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_REPEAT)).kbsc1(KbSc(KB_DOT  , KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_REPEAT)).flag(MENU_HIDDEN);
         v.New().create("Previous Animation", PrevAnim, T).kbsc(KbSc(KB_PGUP, KBSC_CTRL_CMD|KBSC_REPEAT)).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Next Animation"    , NextAnim, T).kbsc(KbSc(KB_PGDN, KBSC_CTRL_CMD|KBSC_REPEAT)).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         break;
      }
   }
   AnimEditor& create()
   {
      super.create(Draw, false, 0, PI, 1, 0.01, 1000); v4.toggleHorizontal();
      flt h=ctrls.rect().h();
      T+=axis      .create(Rect_LU(ctrls     .rect().ld(), h)     ).focusable(false).desc(S+"Display identity matrix axes, where:\nRed = right (x vector)\nGreen = up (y vector)\nBlue = forward (z vector)\nLength of each vector is 1 unit\nPlease note that the camera in Object Editor is by default faced from forward to backward.\n\nKeyboard Shortcut: Alt+A"); axis.mode=BUTTON_TOGGLE; axis.set(true); axis.image="Gui/Misc/axis.img";
      T+=draw_bones.create(Rect_LU(axis      .rect().ru(), h), "B").focusable(false).desc(S+"Display skeleton bones\nKeyboard Shortcut: Alt+B"); draw_bones.mode=BUTTON_TOGGLE; draw_bones.set(true);
      T+=draw_mesh .create(Rect_LU(draw_bones.rect().ru(), h), "M").focusable(false).desc(S+"Display mesh\nKeyboard Shortcut: Alt+M"          ); draw_mesh .mode=BUTTON_TOGGLE; draw_mesh .set(true);
         wire      .pos   (        draw_mesh .rect().ru());
      T+=show_grid .create(Rect_LU(axis      .rect().ld(), h)).focusable(false).desc("Draw grid\nKeyboard Shortcut: Alt+G"); show_grid.mode=BUTTON_TOGGLE; show_grid.set(false); show_grid.image="Gui/Misc/grid.img";
      cam_spherical.hide(); cam_lock.pos(cam_spherical.pos());

      T+=undo  .create(Rect_LU(ctrls.rect().ru()+Vec2(h, 0), 0.05, 0.05)     ).func(Undo  , T).focusable(false).desc("Undo\nKeyboard Shortcut: Ctrl+Z"      ); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU( undo.rect().ru()           , 0.05, 0.05)     ).func(Redo  , T).focusable(false).desc("Redo\nKeyboard Shortcut: Ctrl+Shift+Z"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU( redo.rect().ru()           , 0.05, 0.05), "L").func(Locate, T).focusable(false).desc("Locate this element in the Project");

      T+=play.create(Rect_LU(locate.rect().ru()+Vec2(h, 0), h), true).desc(S+"Play Animation\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+P, "+Kb.ctrlCmdName()+"+D");
      props.New().create("Play Speed", MEMBER(AnimEditor, anim_speed)).precision(2);
      Rect r=AddProperties(props, T, play.rect().ru()+Vec2(0.01, 0), play.rect().h(), 0.15, &ObjEdit.ts); REPAO(props).autoData(this);

      T+=op.create(Rect_LU(r.ru()+Vec2(h, 0), h*OP_NUM, h), 0, (cchar**)null, OP_NUM);
      op.tab(OP_ORN  ).setImage("Gui/Misc/target.img" ).desc(S+"Set Target Orientation KeyFrames\n\nSelect with LeftClick\nTransform with RightClick\nRotate with Alt+RightClick\nHold Shift for more precision\nHold "+Kb.ctrlCmdName()+" to transform all KeyFrames\n\nKeyboard Shortcut: F1");
      op.tab(OP_ORN2 ).setImage("Gui/Misc/target2.img").desc(S+"Set Target Orientation KeyFrames for Bone and its Parent\n\nSelect with LeftClick\nTransform with RightClick\nRotate with Alt+RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: F2");
      op.tab(OP_POS  ).setImage("Gui/Misc/move.img"   ).desc(S+"Set Position Offset KeyFrames\n\nSelect with LeftClick\nTransform with RightClick\nHold Shift for more precision\nHold "+Kb.ctrlCmdName()+" to transform all KeyFrames\nHold Alt to use World Matrix alignment\n\nKeyboard Shortcut: F3");
      op.tab(OP_SCALE).setImage("Gui/Misc/scale.img"  ).desc(S+"Set Scale KeyFrames\n\nSelect with LeftClick\nTransform with RightClick\nHold Shift for more precision\nHold "+Kb.ctrlCmdName()+" to transform all KeyFrames\n\nKeyboard Shortcut: F4");
      Node<MenuElm> n;
      n.New().create("Undo" , Undo, T).kbsc(KbSc(KB_Z, KBSC_CTRL_CMD|KBSC_REPEAT)).flag(MENU_HIDDEN); // keep those hidden because they occupy too much of visible space
      n.New().create("Redo" , Redo, T).kbsc(KbSc(KB_Y, KBSC_CTRL_CMD|KBSC_REPEAT)).kbsc1(KbSc(KB_Z, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_REPEAT)).flag(MENU_HIDDEN); // keep those hidden because they occupy too much of visible space
      n.New().create("Undo2", Undo, T).kbsc(KbSc(KB_BACK, KBSC_ALT           |KBSC_REPEAT)).flag(MENU_HIDDEN); // keep those hidden because they occupy too much of visible space
      n.New().create("Redo2", Redo, T).kbsc(KbSc(KB_BACK, KBSC_ALT|KBSC_SHIFT|KBSC_REPEAT)).flag(MENU_HIDDEN); // keep those hidden because they occupy too much of visible space
      n.New().create("Delete KeyFrame"                 , DelFrame      , T).kbsc(KbSc(KB_DEL, KBSC_CTRL_CMD)).desc("This will delete a single keyframe for selected bone");
      n.New().create("Delete KeyFrames"                , DelFrames     , T).kbsc(KbSc(KB_DEL, KBSC_CTRL_CMD|KBSC_SHIFT)).desc("This will delete all keyframes for selected bone");
      n.New().create("Delete All Bone KeyFrames at End", DelFramesAtEnd, T).kbsc(KbSc(KB_DEL, KBSC_CTRL_CMD|KBSC_WIN_CTRL)).desc("This will delete keyframes located at the end of the animation, for all bones (except root motion).");
      n++;
      n.New().create("Freeze Delete KeyFrame"          , FreezeDelFrame , T).kbsc(KbSc(KB_DEL, KBSC_ALT           )).desc("This will delete a single keyframe for selected bone, without affecting transforms of other bones");
      n.New().create("Freeze Delete KeyFrames"         , FreezeDelFrames, T).kbsc(KbSc(KB_DEL, KBSC_ALT|KBSC_SHIFT)).desc("This will delete all keyframes for selected bone, without affecting transforms of other bones");
      n++;
      n.New().create("Reverse KeyFrames", ReverseFrames, T).kbsc(KbSc(KB_R, KBSC_CTRL_CMD|KBSC_SHIFT)); // avoid Ctrl+R collision with reload project element
      n.New().create("Apply Speed"      , ApplySpeed   , T).kbsc(KbSc(KB_S, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_ALT)); // avoid Ctrl+R collision with reload project element
      n++;
      n.New().create("Reduce KeyFrames"           , Optimize   , T).kbsc(KbSc(KB_O, KBSC_CTRL_CMD));
      n.New().create("Scale Position Keys"        , ScalePosKey, T).kbsc(KbSc(KB_S, KBSC_CTRL_CMD));
      n.New().create("Change Speed for Time Range", TimeRangeSp, T).kbsc(KbSc(KB_S, KBSC_CTRL_CMD|KBSC_SHIFT));
      n++;
      n.New().create("Freeze Bone", FreezeBone, T).desc("This option will adjust position offset to the root bone, so that currently selected bone will appear without movement.").kbsc(KbSc(KB_F, KBSC_CTRL_CMD|KBSC_ALT));
      n++;
      n.New().create("Set Mirrored from Selection", SetMirrorSel, T).kbsc(KbSc(KB_M, KBSC_CTRL_CMD           )).desc("This option will set bone animation from the other side as mirrored version of the selected bone");
      n.New().create("Set Selection from Mirrored", SetSelMirror, T).kbsc(KbSc(KB_M, KBSC_CTRL_CMD|KBSC_SHIFT)).desc("This option will set selected bone animation as mirrored version of the bone from the other side");
      n++;
      n.New().create( "Copy Bone Position"         , SkelBonePosCopy  , T).kbsc(KbSc(KB_C, KBSC_CTRL_CMD));
      n.New().create( "Copy Bone Position Relative", SkelBonePosCopyR , T).kbsc(KbSc(KB_C, KBSC_CTRL_CMD|KBSC_SHIFT)).desc("Copy selected bone position relative to root");
      n.New().create("Paste Bone Position"         , SkelBonePosPaste , T).kbsc(KbSc(KB_V, KBSC_CTRL_CMD));
      n.New().create("Paste Bone Position Z"       , SkelBonePosPasteZ, T).kbsc(KbSc(KB_V, KBSC_CTRL_CMD|KBSC_SHIFT));
      n.New().create("Paste Bone Position X"       , SkelBonePosPasteX, T).kbsc(KbSc(KB_V, KBSC_CTRL_CMD|KBSC_WIN_CTRL));
      n.New().create("Paste Bone Position Y"       , SkelBonePosPasteY, T).kbsc(KbSc(KB_V, KBSC_CTRL_CMD|KBSC_ALT));
      n++;
      n.New().create("Set Root From Body Z"      , RootFromBodyZ , T).kbsc(KbSc(KB_B, KBSC_CTRL_CMD));
      n.New().create("Set Root From Body X"      , RootFromBodyX , T).kbsc(KbSc(KB_B, KBSC_CTRL_CMD|KBSC_SHIFT));
      n.New().create("Set Root From Body XZ"     , RootFromBodyXZ, T).kbsc(KbSc(KB_B, KBSC_CTRL_CMD|KBSC_ALT));
      n.New().create("Del Root Position+Rotation", RootDel       , T).kbsc(KbSc(KB_R, KBSC_CTRL_CMD|KBSC_ALT));
      n++;
      n.New().create("Mirror"   , Mirror, T).desc("Mirror entire animation along X axis");
      n.New().create("Rotate X" , RotX  , T).kbsc(KbSc(KB_X, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT)).desc("Rotate entire animation along X axis (hold Shift for half rotation)");
      n.New().create("Rotate Y" , RotY  , T).kbsc(KbSc(KB_Y, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT)).desc("Rotate entire animation along Y axis (hold Shift for half rotation)");
      n.New().create("Rotate Z" , RotZ  , T).kbsc(KbSc(KB_Z, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT)).desc("Rotate entire animation along Z axis (hold Shift for half rotation)");
      n.New().create("Rotate XH", RotXH , T).kbsc(KbSc(KB_X, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT|KBSC_SHIFT)).desc("Rotate entire animation along X axis (hold Shift for half rotation)").flag(MENU_HIDDEN);
      n.New().create("Rotate YH", RotYH , T).kbsc(KbSc(KB_Y, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT|KBSC_SHIFT)).desc("Rotate entire animation along Y axis (hold Shift for half rotation)").flag(MENU_HIDDEN);
      n.New().create("Rotate ZH", RotZH , T).kbsc(KbSc(KB_Z, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT|KBSC_SHIFT)).desc("Rotate entire animation along Z axis (hold Shift for half rotation)").flag(MENU_HIDDEN);
      n++;
      n.New().create("Transform Original Object", TransformObj, T);
      T+=cmd.create(Rect_LU(op.rect().ru(), h), n).focusable(false); cmd.flag|=COMBOBOX_CONST_TEXT;

      T+=edit.create(Rect_LU(cmd.rect().ru()+Vec2(h, 0), 0.12, h), "Edit").func(Fullscreen, T).focusable(false).desc(S+"Close Editing\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+E");
      cchar8 *settings_t[]={"Settings"};
      T+=settings.create(Rect_LU(edit.rect().ru()+Vec2(h, 0), 0.17, h), 0, settings_t, Elms(settings_t));
      settings.tab(0).desc("Keyboard Shortcut: Alt+S")+=settings_region.create(Rect_LU(settings.rect().ru(), 0.333+h*3, 0.5)).skin(&TransparentSkin, false); settings_region.kb_lit=false;
      {
         flt x=0.01, y=-0.005, l=h*1.1, ll=l*1.1, w=0.25, ph=h*0.95;
         ts.reset().size=0.041;
         settings_region+=t_settings.create(Vec2(settings_region.clientWidth()/2, y-h/2), "Settings also affect reloading", &ts); y-=ll;
         settings_region+=loop  .create(Rect_U(settings_region.clientWidth()*(1.0/3.5), y, 0.15, h), "Loop"  ).func(Loop  , T); loop  .mode=BUTTON_TOGGLE;
         settings_region+=linear.create(Rect_U(settings_region.clientWidth()*(2.5/3.5), y, 0.15, h), "Linear").func(Linear, T); linear.mode=BUTTON_TOGGLE;
         y-=ll;
         settings_region+=t_settings_root.create(Vec2(x, y-h/2), "Root:", &ObjEdit.ts); y-=l;
         settings_region+=root_del_pos  .create(Rect_LU(x, y, w, h), "Del Position").func(RootDelPos, T);
         settings_region+=root_del_pos_x.create(Rect_LU(root_del_pos  .rect().ru()+Vec2(0.01, 0), h, h), "X").func(RootDelPosX, T).subType(BUTTON_TYPE_TAB_LEFT      ); root_del_pos_x.mode=BUTTON_TOGGLE;
         settings_region+=root_del_pos_y.create(Rect_LU(root_del_pos_x.rect().ru()              , h, h), "Y").func(RootDelPosY, T).subType(BUTTON_TYPE_TAB_HORIZONTAL); root_del_pos_y.mode=BUTTON_TOGGLE;
         settings_region+=root_del_pos_z.create(Rect_LU(root_del_pos_y.rect().ru()              , h, h), "Z").func(RootDelPosZ, T).subType(BUTTON_TYPE_TAB_RIGHT     ); root_del_pos_z.mode=BUTTON_TOGGLE;
         y-=l;
         settings_region+=root_del_rot  .create(Rect_LU(x, y, w, h), "Del Rotation").func(RootDelRot, T);
         settings_region+=root_del_rot_x.create(Rect_LU(root_del_rot  .rect().ru()+Vec2(0.01, 0), h, h), "X").func(RootDelRotX, T).subType(BUTTON_TYPE_TAB_LEFT      ); root_del_rot_x.mode=BUTTON_TOGGLE;
         settings_region+=root_del_rot_y.create(Rect_LU(root_del_rot_x.rect().ru()              , h, h), "Y").func(RootDelRotY, T).subType(BUTTON_TYPE_TAB_HORIZONTAL); root_del_rot_y.mode=BUTTON_TOGGLE;
         settings_region+=root_del_rot_z.create(Rect_LU(root_del_rot_y.rect().ru()              , h, h), "Z").func(RootDelRotZ, T).subType(BUTTON_TYPE_TAB_RIGHT     ); root_del_rot_z.mode=BUTTON_TOGGLE;
         y-=l;
       //settings_region+=root_del_scale .create(Rect_LU(x, y, w, h), "Del Scale"   ).func(RootDelScale, T); root_del_scale.mode=BUTTON_TOGGLE; y-=l;
         settings_region+=root_from_body .create(Rect_LU(x, y, w, h), "From Body"   ).func(RootFromBody, T); root_from_body.mode=BUTTON_TOGGLE; y-=l;
       //settings_region+=root_2_keys    .create(Rect_LU(x, y, w, h), "2 Keys Only" ).func(Root2Keys   , T); root_2_keys   .mode=BUTTON_TOGGLE; y-=l; root_2_keys.desc("Limit number of keyframes to 2 keys only: Start+End");
         settings_region+=root_smooth    .create(Rect_LU(x, y, w, h), "Smooth"      ).func(RootSmooth  , T); root_smooth.desc("Set smooth root rotation and movement with constant velocities");
         settings_region+=root_smooth_rot.create(Rect_LU(root_smooth    .rect().ru()+Vec2(0.01, 0), h*2, h), "Rot").func(RootSmoothRot, T).subType(BUTTON_TYPE_TAB_LEFT ); root_smooth_rot.mode=BUTTON_TOGGLE; root_smooth_rot.desc("Set smooth root rotation with constant velocities");
         settings_region+=root_smooth_pos.create(Rect_LU(root_smooth_rot.rect().ru()              , h*2, h), "Pos").func(RootSmoothPos, T).subType(BUTTON_TYPE_TAB_RIGHT); root_smooth_pos.mode=BUTTON_TOGGLE; root_smooth_pos.desc("Set smooth root movement with constant velocities");
         y-=l;
         settings_region+=root_set_move  .create(Rect_LU(x, y, w, h), "Set Movement").func(RootSetMove , T); root_set_move .mode=BUTTON_TOGGLE; root_set_move.desc("Override Animation's root movement with a custom value");
         settings_region+=root_set_rot   .create(Rect_LU(x, y-3*ph, w, h), "Set Rotation").func(RootSetRot  , T); root_set_rot  .mode=BUTTON_TOGGLE; root_set_rot.desc("Override Animation's root rotation with a custom value");
       //settings_region+=reload         .create(Rect_U (settings_region.clientWidth()/2, y-6*ph-0.01, 0.18, h+0.005), "Reload").func(Reload, T);
         settings_region+=reload         .create(Rect_LD(x, y-6*ph, 0.18, h+0.005), "Reload").func(Reload, T);
         root_props.New().create("X", MemberDesc(DATA_REAL).setFunc(RootMoveX, RootMoveX));
         root_props.New().create("Y", MemberDesc(DATA_REAL).setFunc(RootMoveY, RootMoveY));
         root_props.New().create("Z", MemberDesc(DATA_REAL).setFunc(RootMoveZ, RootMoveZ));
         root_props.New().create("X", MemberDesc(DATA_REAL).setFunc(RootRotX , RootRotX )).mouseEditSpeed(15).precision(1);
         root_props.New().create("Y", MemberDesc(DATA_REAL).setFunc(RootRotY , RootRotY )).mouseEditSpeed(15).precision(1);
         root_props.New().create("Z", MemberDesc(DATA_REAL).setFunc(RootRotZ , RootRotZ )).mouseEditSpeed(15).precision(1);
         Rect r=AddProperties(root_props, settings_region, Vec2(x+w+0.01, y), ph, 0.17, &ObjEdit.ts); REPAO(root_props).autoData(this); y=Min(r.min.y, reload.rect().min.y);
         settings_region.size(Vec2(settings_region.rect().w(), -y+0.02));
      }

      T+=start     .create("[").focusable(false).func(Start    , T).desc(S+"Go to animation start\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+Home, "+Kb.ctrlCmdName()+"+[");
      T+=prev_frame.create("<").focusable(false).func(PrevFrame, T).desc(S+"Go to previous KeyFrame\nHold Shift to iterate all KeyFrame types\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+LeftArrow, "+Kb.ctrlCmdName()+"+<");
      T+=force_play.create(   ).focusable(false)                   .desc(S+"Play as long as this button is pressed\nHold Shift to play backwards\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+W"); force_play.image=Proj.icon_play; force_play.mode=BUTTON_CONTINUOUS;
      T+=next_frame.create(">").focusable(false).func(NextFrame, T).desc(S+"Go to next KeyFrame\nHold Shift to iterate all KeyFrame types\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+RightArrow, "+Kb.ctrlCmdName()+"+>");
      T+=end       .create("]").focusable(false).func(End      , T).desc(S+"Go to animation end\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+End, "+Kb.ctrlCmdName()+"+]");
      T+=track           .create(false);
      T+=optimize_anim   .create();
      T+=scale_pos_keys  .create();
      T+=time_range_speed.create();
      preview.create();
      return T;
   }
   SkelBone transformedBone(int i)C
   {
      SkelBone bone;
      if(skel && InRange(i, skel.bones))bone=skel.bones[i];//else bone is already set to identity in constructor
      bone*=anim_skel.boneRoot(i).matrix();
      return bone;
   }
   Matrix transformedBoneAxis(int i)C
   {
      Matrix m=transformedBone(i);
      if(op()==OP_POS && Kb.alt())m.orn().identity();
      m.scaleOrn((i>=0) ? SkelSlotSize : 1.0f/3);
      return m;
   }
   int getBone(GuiObj *go, C Vec2 &screen_pos)
   {
      int index=-1; if(Edit.Viewport4.View *view=v4.getView(go))
      {
         view.setViewportCamera();
         flt dist=0;
         REPA(anim_skel.bones)
         {
            SkelBone bone=transformedBone(i);
            flt d; if(Distance2D(screen_pos, Edge(bone.pos, bone.to()), d, 0))if(index<0 || d<dist){index=i; dist=d;}
         }
         if(dist>0.03)index=-1;
      }
      return index;
   }
   int boneParent(int bone)C
   {
      return skel ? skel.boneParent(bone) : -1;
   }
   AnimKeys* findKeys(Animation *anim, int sbon_index, bool root=true)
   {
      if(anim)
      {
         if(skel)if(C SkelBone *sbon=skel.bones.addr(sbon_index))return anim.findBone(sbon.name, sbon.type, sbon.type_index, sbon.type_sub); // use types in case animation was from another skeleton and we haven't adjusted types
         if(root)return &anim.keys;
      }
      return null;
   }
   AnimKeys* findVisKeys(int sbon_index, bool root=true) {return findKeys(getVisAnim(), sbon_index, root);}
   AnimKeys* findKeys   (int sbon_index, bool root=true) {return findKeys(      anim  , sbon_index, root);}
   AnimKeys*  getKeys   (int sbon_index, bool root=true)
   {
      if(anim)
      {
         if(skel)if(C SkelBone *sbon=skel.bones.addr(sbon_index))return &anim.getBone(sbon.name, sbon.type, sbon.type_index, sbon.type_sub); // use types in case animation was from another skeleton and we haven't adjusted types
         if(root)return &anim.keys;
      }
      return null;
   }
   AnimKeys* findKeysParent(int sbon_index, bool root=true) {return sbon_index<0 ? null : findKeys(boneParent(sbon_index), root);} // return null for root, because 'findKeys' already returns root for -1, and 'findKeysParent' would return the same

   static int CompareKey(C AnimKeys.Orn   &key, C flt &time) {return Compare(key.time, time);}
   static int CompareKey(C AnimKeys.Pos   &key, C flt &time) {return Compare(key.time, time);}
   static int CompareKey(C AnimKeys.Scale &key, C flt &time) {return Compare(key.time, time);}

   static AnimKeys.Orn  * FindOrn  (AnimKeys &keys, flt time) {AnimKeys.Orn   *key=null; flt dist; REPA(keys.orns  ){flt d=Abs(keys.orns  [i].time-time); if(!key || d<dist){key=&keys.orns  [i]; dist=d;}} return (!key || dist>TimeEps()) ? null : key;}
   static AnimKeys.Pos  * FindPos  (AnimKeys &keys, flt time) {AnimKeys.Pos   *key=null; flt dist; REPA(keys.poss  ){flt d=Abs(keys.poss  [i].time-time); if(!key || d<dist){key=&keys.poss  [i]; dist=d;}} return (!key || dist>TimeEps()) ? null : key;}
   static AnimKeys.Scale* FindScale(AnimKeys &keys, flt time) {AnimKeys.Scale *key=null; flt dist; REPA(keys.scales){flt d=Abs(keys.scales[i].time-time); if(!key || d<dist){key=&keys.scales[i]; dist=d;}} return (!key || dist>TimeEps()) ? null : key;}

   static AnimKeys.Orn  & GetOrn  (AnimKeys &keys, flt time, C Orient &Default) {if(AnimKeys.Orn   *key=FindOrn  (keys, time))return *key; int i; if(keys.orns  .binarySearch(time, i, CompareKey))return keys.orns  [i]; AnimKeys.Orn   &key=keys.orns  .NewAt(i); key.time=time; key.orn  =Default; return key;}
   static AnimKeys.Pos  & GetPos  (AnimKeys &keys, flt time, C Vec    &Default) {if(AnimKeys.Pos   *key=FindPos  (keys, time))return *key; int i; if(keys.poss  .binarySearch(time, i, CompareKey))return keys.poss  [i]; AnimKeys.Pos   &key=keys.poss  .NewAt(i); key.time=time; key.pos  =Default; return key;}
   static AnimKeys.Scale& GetScale(AnimKeys &keys, flt time, C Vec    &Default) {if(AnimKeys.Scale *key=FindScale(keys, time))return *key; int i; if(keys.scales.binarySearch(time, i, CompareKey))return keys.scales[i]; AnimKeys.Scale &key=keys.scales.NewAt(i); key.time=time; key.scale=Default; return key;}

   flt getBlend(C AnimKeys.Key &key )C {return getBlend(key.time);}
   flt getBlend(           flt  time)C
   {
      flt delta=time-animTime();
      if(anim && anim.loop())
      {
         delta=Frac(delta, anim.length());
         if(delta>anim.length()/2)delta-=anim.length();
      }
      return BlendSmoothCube(delta/blend_range);
   }

   static void ScaleScaleFactor(Vec &scale_factor, C Vec &scale) {scale_factor=ScaleFactorR(ScaleFactor(scale_factor)*scale);}

   virtual void update(C GuiPC &gpc)override
   {
      lit_bone=-1;
      if(gpc.visible && visible())
      {
         optimize_anim .refresh(); // refresh all the time because animation can be changed all the time (since we're accessing it directly from 'Animations' cache)
         scale_pos_keys.refresh(); // refresh all the time because animation can be changed all the time (since we're accessing it directly from 'Animations' cache)
         playUpdate();
         setAnimSkel();

         if(draw_bones())
         {
            // get lit
            lit_bone=getBone(Gui.ms(), Ms.pos());

            // get 'bone_axis'
            if((op()==OP_POS || op()==OP_SCALE)
         // && skel && InRange(sel_bone, skel.bones) don't check this so we can process just root too
            )MatrixAxis(v4, transformedBoneAxis(sel_bone), bone_axis);else bone_axis=-1;

            // select / edit
            if(Edit.Viewport4.View *view=v4.getView(Gui.ms()))
            {
               if(Ms.bp(0))selBone(lit_bone);else
               if(Ms.b (1) && op()>=0
            //&& skel && InRange(sel_bone, skel.bones) don't check this so we can process just root too
               )if(AnimKeys *keys=getKeys(sel_bone))
               {
                  view.setViewportCamera();
                    C SkelBone * sbon=(skel ? skel.bones.addr(sel_bone) : null);
                      SkelBone   bone=  transformedBone      (sel_bone), bone_parent; if(sel_bone>=0)bone_parent=transformedBone(boneParent(sel_bone)); // set 'bone_parent' only if we have a bone selected, otherwise (when root is selected) then keep it as identity
                C AnimSkelBone &asbon=   anim_skel.boneRoot  (sel_bone);
                  Vec2           mul =(Kb.shift() ? 0.1 : 1.0)*0.5;
                  bool           all =Kb.ctrlCmd(), rotate=Kb.alt(), use_blend=(blend_range>0);
                  switch(op())
                  {
                     case OP_ORN:
                     {
                        undos.set("orn");
                     op_orn:

                      //Orient bone_orn=GetAnimOrient(bone, &bone_parent); bone_orn.fix();
                        Orient bone_orn=asbon.orn; if(!bone_orn.fix()){if(sbon /*&& skel - no need to check because 'sbon' already guarantees 'skel'*/)bone_orn=GetAnimOrient(*sbon, skel.bones.addr(sbon.parent));else bone_orn.identity();} // 'asbon.orn' can be zero
                        AnimKeys.Orn *orn=((all && keys.orns.elms()) ? null : &GetOrn(*keys, animTime(), bone_orn)); // if there are no keys then create
                        if(rotate)
                        {
                           flt d=(Ms.d()*mul).sum();
                           if(all)
                           {
                              if(use_blend)REPA (keys.orns)keys.orns[i].orn.rotateDir(getBlend(keys.orns[i])*d);
                              else         REPAO(keys.orns).orn.rotateDir(d);
                           }else // single
                           {
                              orn.orn.rotateDir(d);
                           }
                        }else
                        {
                           mul*=CamMoveScale(v4.perspective())*MoveScale(*view);
                           Vec d=ActiveCam.matrix.x*Ms.d().x*mul.x
                                +ActiveCam.matrix.y*Ms.d().y*mul.y;
                           orn_target+=d;

                           bool freeze    =Kb.b(KB_H); // hold
                           bool freeze_pos=Kb.b(KB_F);
                           if(freeze || freeze_pos || all)
                           {
                              Vec     p=orn_target-bone.pos; p/=Matrix3(bone_parent); p.normalize();
                              Matrix3 transform;
                              if(freeze || freeze_pos)
                              {
                                 if(skel)
                                 {
                                    Orient next=bone_orn; next.rotateToDir(p); next.fixPerp();
                                    GetTransform(transform, bone_orn, next);
                                    anim.freezeRotate(*skel, sel_bone, all ? -1 : keys.orns.index(orn), transform, freeze_pos);
                                 }
                              }else
                              if(all)
                              {
                                 if(use_blend)REPA(keys.orns)
                                 {
                                    Orient next=bone_orn; next.rotateToDir(p, getBlend(keys.orns[i])); next.fixPerp();
                                    GetTransform(transform, bone_orn, next);
                                    keys.orns[i].orn.mul(transform, true).fix();
                                 }else
                                 {
                                    Orient next=bone_orn; next.rotateToDir(p); next.fixPerp();
                                    GetTransform(transform, bone_orn, next);
                                    REPAO(keys.orns).orn.mul(transform, true).fix();
                                 }
                              }
                           }else // single
                           {
                              bone.setFromTo(bone.pos, orn_target);
                              orn.orn=GetAnimOrient(bone, &bone_parent); orn.orn.fix();
                           }
                        }
                        keys.setTangents(anim.loop(), anim.length());
                        anim.setRootMatrix();
                        setChanged();
                     }break;

                     case OP_ORN2:
                     {
                        undos.set("orn");
                        if(!sbon /*|| !skel - no need to check because 'sbon' already guarantees 'skel'*/)goto op_orn;
                      C SkelBone *sbon_parent=skel.bones.addr(sbon.parent);
                        AnimKeys *keys_parent=        getKeys(sbon.parent, false);
                        if(!sbon_parent || !keys_parent)goto op_orn;

                        if(!rotate)
                        {
                           Vec perp=bone.pos-NearestPointOnLine(bone.pos, bone_parent.pos, !(orn_target-bone_parent.pos));
                           if( perp.normalize()<=EPS)switch(sbon.type)
                           {
                              case BONE_FINGER:
                              {
                                 int hand=skel.findParent(sel_bone, BONE_HAND);
                                 if( hand>=0)perp=transformedBone(hand).perp;else perp.set(0, 1, 0);
                              }break;

                              case BONE_HEAD     :
                              case BONE_FOOT     :
                              case BONE_SPINE    :
                              case BONE_NECK     : perp.set(0, 0, bone.pos.z-orn_target.z); break;

                              case BONE_TOE      : perp.set(0, bone.pos.y-orn_target.y, 0); break;

                              case BONE_UPPER_ARM:
                              case BONE_FOREARM  : perp.set(0, 0, -1); break;

                              case BONE_LOWER_LEG: perp.set(0, 0,  1); break;

                              default            : perp=orn_perp; break;
                           }

                           mul*=CamMoveScale(v4.perspective())*MoveScale(*view);
                           Vec d=ActiveCam.matrix.x*Ms.d().x*mul.x
                                +ActiveCam.matrix.y*Ms.d().y*mul.y;
                           orn_target+=d;

                           Vec desired_dir=orn_target-bone_parent.pos;

                           flt a_length=Dist(bone_parent.pos, bone.pos), b_length=bone.length,
                             cur_length=Dist(bone_parent.pos, bone.to()),
                         desired_length=Min (desired_dir.length(), a_length+b_length);

                           // first rotate the parent to match the new direction (from parent->child.to to parent->orn_target)
                           Matrix3 m;
                           m.setRotation(!(bone.to()-bone_parent.pos), !desired_dir);
                           SCAST(Orient, bone_parent)*=m;
                           SCAST(Orient, bone       )*=m; // this should include position, but we don't actually need it
                                         perp        *=m;

                           // rotate the parent and bone
                           Vec rotate_axis=CrossN(desired_dir, perp);

                           flt cur_angle=TriABAngle(    cur_length, a_length, b_length),
                           desired_angle=TriABAngle(desired_length, a_length, b_length);
                           flt     delta=desired_angle-cur_angle;
                           SCAST(Orient, bone_parent)*=Matrix3().setRotate(rotate_axis, delta);
                         //SCAST(Orient, bone       )*=Matrix3().setRotate(rotate_axis, delta); child should be transformed too, but instead we're transforming child by 'delta' below

                               cur_angle=TriABAngle(a_length, b_length,     cur_length);
                           desired_angle=TriABAngle(a_length, b_length, desired_length);
                           SCAST(Orient, bone       )*=Matrix3().setRotate(rotate_axis, delta + desired_angle-cur_angle); // add delta because child is transformed by parent

                           Orient pose=GetAnimOrient(bone_parent, &NoTemp(transformedBone(sbon_parent.parent))); pose.fix();
                           AnimKeys.Orn *orn=&GetOrn(*keys_parent, animTime(), pose);
                           orn.orn=pose;

                           pose=GetAnimOrient(bone, &bone_parent); pose.fix();
                           orn=&GetOrn(*keys, animTime(), pose);
                           orn.orn=pose;
                        }else
                        {
                           flt d=(Ms.d()*mul).sum();

                           Matrix3 m;
                           m.setRotate(!(bone.to()-bone_parent.pos), d);
                           SCAST(Orient, bone_parent)*=m;
                           SCAST(Orient, bone       )*=m; // this should include position, but we don't actually need it
                           
                           Orient pose=GetAnimOrient(bone_parent, &NoTemp(transformedBone(sbon_parent.parent))); pose.fix();
                           AnimKeys.Orn *orn=&GetOrn(*keys_parent, animTime(), pose);
                           orn.orn=pose;

                           pose=GetAnimOrient(bone, &bone_parent); pose.fix();
                           orn=&GetOrn(*keys, animTime(), pose);
                           orn.orn=pose;
                        }
                        keys_parent.setTangents(anim.loop(), anim.length());
                        keys       .setTangents(anim.loop(), anim.length());
                        anim.setRootMatrix();
                        setChanged();
                     }break;

                     case OP_POS:
                     {
                        Ms.freeze();
                        undos.set("pos");
                        AnimKeys.Pos *pos=((all && keys.poss.elms()) ? null : &GetPos(*keys, animTime(), asbon.pos)); // if there are no keys then create
                        mul*=CamMoveScale(v4.perspective())*MoveScale(*view);
                        Matrix m=transformedBoneAxis(sel_bone);
                        Vec    d; switch(bone_axis)
                        {
                           case  0: d=AlignDirToCam(m.x, Ms.d()  *mul ); break;
                           case  1: d=AlignDirToCam(m.y, Ms.d()  *mul ); break;
                           case  2: d=AlignDirToCam(m.z, Ms.d()  *mul ); break;
                           default: d=ActiveCam.matrix.x*Ms.d().x*mul.x
                                     +ActiveCam.matrix.y*Ms.d().y*mul.y; break;
                        }
                        orn_target+=d;
                        d/=Matrix3(bone_parent);
                        if(Kb.b(KB_F))
                        {
                           if(skel)anim.freezeMove(*skel, sel_bone, all ? -1 : keys.poss.index(pos), d);
                        }else
                        if(all)
                        {
                           if(use_blend)REPA (keys.poss)keys.poss[i].pos+=d*getBlend(keys.poss[i]);
                           else         REPAO(keys.poss).pos+=d;
                        }else // single
                        {
                           pos.pos+=d;
                        }
                        keys.setTangents(anim.loop(), anim.length());
                        anim.setRootMatrix();
                        setChanged();
                     }break;

                     case OP_SCALE:
                     {
                        Ms.freeze();
                        undos.set("scale");
                        AnimKeys.Scale *scale=((all && keys.scales.elms()) ? null : &GetScale(*keys, animTime(), asbon.scale)); // if there are no keys then create
                        Vec d=0;
                        switch(bone_axis)
                        {
                           case  0: d.x+=AlignDirToCamEx(bone.cross(), Ms.d()*mul)      ; break;
                           case  1: d.y+=AlignDirToCamEx(bone.perp   , Ms.d()*mul)      ; break;
                           case  2: d.z+=AlignDirToCamEx(bone.dir    , Ms.d()*mul)      ; break;
                           default: d  +=                             (Ms.d()*mul).sum(); break;
                        }
                        Vec mul=ScaleFactor(d);
                        if(Kb.ctrlCmd()) // all
                        {
                           if(use_blend)REPA(keys.scales)ScaleScaleFactor(keys.scales[i].scale, ScaleFactor(d*getBlend(keys.scales[i])));
                           else         REPA(keys.scales)ScaleScaleFactor(keys.scales[i].scale, mul);
                        }else // single
                        {
                           ScaleScaleFactor(scale.scale, mul);
                        }
                        keys.setTangents(anim.loop(), anim.length());
                        anim.setRootMatrix();
                        setChanged();
                        setOrnTarget();
                     }break;
                  }
               }
               if(Kb.ctrlCmd() && Kb.shift() && Kb.b(KB_BACK)) // fast delete of highlighted bone keyframes
                  if(lit_bone>=0) // only if have any highlighted bone (to skip root)
                     delFrames(lit_bone);
            }
         }
         if(Ms.bp(2)) // close on middle click
         {
            if(settings_region.contains(Gui.ms()))settings.set(-1);
         }
      }
      super.update(gpc); // process after adjusting 'animTime' so clicking on anim track will not be changed afterwards
   }
   bool selectionZoom(flt &dist)
   {
      flt size=(mesh ? mesh->ext.size().avg() : 0);
      if( size>0)
      {
         dist=size/Tan(v4.perspFov()/2);
         return true;
      }
      return false;
   }
   bool getHit(GuiObj *go, C Vec2 &screen_pos, Vec &hit_pos)
   {
      int hit_part=-1;
      if(mesh)if(Edit.Viewport4.View *view=v4.getView(go))
      {
       C MeshLod  &lod=mesh->getDrawLod(anim_skel.matrix());
         MeshPart  part;
         view.setViewportCamera();
         Vec pos, dir; ScreenToPosDir(screen_pos, pos, dir);
         pos+=(D.viewFrom ()/Dot(dir, ActiveCam.matrix.z))*dir;
         dir*= D.viewRange();
         flt frac, f; Vec hp;
         REPA(lod)
            if(!(lod.parts[i].part_flag&MSHP_HIDDEN))
         {
            part.create(lod.parts[i]).setBase().base.animate(anim_skel);
            if(Sweep(pos, dir, part, null, &f, &hp))if(hit_part<0 || f<frac){hit_part=i; frac=f; hit_pos=hp;}
         }
      }
      return hit_part>=0;
   }
   virtual void camCenter(bool zoom)override
   {
      Vec hit_pos; bool hit=getHit(Gui.ms(), Ms.pos(), hit_pos); flt dist;
      v4.moveTo(hit ? hit_pos :
                (skel && InRange(lit_bone, skel.bones)) ? transformedBone(lit_bone).center() :
                (skel && InRange(sel_bone, skel.bones)) ? transformedBone(sel_bone).center() :
                mesh ? mesh->ext.center :
                VecZero);
      if(zoom && selectionZoom(dist))v4.dist(dist);
   }
   virtual void resize()override
   {
      super.resize();
      track.rect(Rect(Proj.visible() ? 0 : Misc.rect().w(), -clientHeight(), v4.rect().max.x, 0.09-clientHeight()));
      end       .rect(Rect_RD(track     .rect().ru(), 0.07, 0.07));
      next_frame.rect(Rect_RD(end       .rect().ld(), 0.08, 0.07));
      force_play.rect(Rect_RD(next_frame.rect().ld(), 0.08, 0.07));
      prev_frame.rect(Rect_RD(force_play.rect().ld(), 0.08, 0.07));
      start     .rect(Rect_RD(prev_frame.rect().ld(), 0.07, 0.07));
      v4.rect(Rect(v4.rect().min.x, Proj.visible() ? track.rect().max.y : Misc.rect().h()-clientHeight(), v4.rect().max.x, v4.rect().max.y));
      optimize_anim .move(Vec2(rect().w(), rect().h()/-2)-optimize_anim .rect().right());
      scale_pos_keys.move(Vec2(rect().w(), rect().h()/-2)-scale_pos_keys.rect().right());
   }
   void frame(int d)
   {
      if(d)if(AnimKeys *keys=findKeys(sel_bone))switch(Kb.shift() ? -1 : op())
      {
         case OP_ORN :
         case OP_ORN2: if(keys.orns.elms())
         {
            int i=0; for(; i<keys.orns.elms(); i++){flt t=keys.orns[i].time; if(Equal(animTime(), t))break; if(t>=animTime()){if(d>0)d--; break;}}
            i+=d; i=Mod(i, keys.orns.elms()); animTime(keys.orns[i].time);
         }break;

         case OP_POS: if(keys.poss.elms())
         {
            int i=0; for(; i<keys.poss.elms(); i++){flt t=keys.poss[i].time; if(Equal(animTime(), t))break; if(t>=animTime()){if(d>0)d--; break;}}
            i+=d; i=Mod(i, keys.poss.elms()); animTime(keys.poss[i].time);
         }break;

         case OP_SCALE: if(keys.scales.elms())
         {
            int i=0; for(; i<keys.scales.elms(); i++){flt t=keys.scales[i].time; if(Equal(animTime(), t))break; if(t>=animTime()){if(d>0)d--; break;}}
            i+=d; i=Mod(i, keys.scales.elms()); animTime(keys.scales[i].time);
         }break;
         
         default:
         {
            Memt<flt, 16384> times; keys.includeTimes(times, times, times);
            if(times.elms())
            {
               int i=0; for(; i<times.elms(); i++){flt t=times[i]; if(Equal(animTime(), t))break; if(t>=animTime()){if(d>0)d--; break;}}
               i+=d; i=Mod(i, times.elms()); animTime(times[i]);
            }
         }break;
      }
   }

   bool delFrameOrn(int bone)
   {
      if(AnimKeys *keys=findKeys(bone))if(AnimKeys.Orn *key=FindOrn(*keys, animTime()))
      {
         undos.set("del"); keys.orns.removeData(key, true);
         if(keys.is())keys.setTangents(anim.loop(), anim.length());else anim.bones.removeData(static_cast<AnimBone*>(keys), true);
         return true;
      }
      return false;
   }
   bool delFramePos(int bone)
   {
      if(AnimKeys *keys=findKeys(bone))if(AnimKeys.Pos *key=FindPos(*keys, animTime()))
      {
         undos.set("del"); keys.poss.removeData(key, true);
         if(keys.is())keys.setTangents(anim.loop(), anim.length());else anim.bones.removeData(static_cast<AnimBone*>(keys), true);
         return true;
      }
      return false;
   }
   bool delFrameScale(int bone)
   {
      if(AnimKeys *keys=findKeys(bone))if(AnimKeys.Scale *key=FindScale(*keys, animTime()))
      {
         undos.set("del"); keys.scales.removeData(key, true);
         if(keys.is())keys.setTangents(anim.loop(), anim.length());else anim.bones.removeData(static_cast<AnimBone*>(keys), true);
         return true;
      }
      return false;
   }

   bool delFramesOrn(int bone)
   {
      if(AnimKeys *keys=findKeys(bone))if(keys.orns.elms()){undos.set("delAll"); keys.orns.del(); if(!keys.is())anim.bones.removeData(static_cast<AnimBone*>(keys), true); return true;}
      return false;
   }
   bool delFramesPos(int bone)
   {
      if(AnimKeys *keys=findKeys(bone))if(keys.poss.elms()){undos.set("delAll"); keys.poss.del(); if(!keys.is())anim.bones.removeData(static_cast<AnimBone*>(keys), true); return true;}
      return false;
   }
   bool delFramesScale(int bone)
   {
      if(AnimKeys *keys=findKeys(bone))if(keys.scales.elms()){undos.set("delAll"); keys.scales.del(); if(!keys.is())anim.bones.removeData(static_cast<AnimBone*>(keys), true); return true;}
      return false;
   }

   bool freezeDelFramePos(int bone)
   {
      if(skel)if(AnimKeys *keys=findKeys(bone))if(AnimKeys.Pos *key=FindPos(*keys, animTime())){undos.set("del"); anim.freezeDelPos(*skel, bone, keys.poss.index(key)); return true;}
      return false;
   }
   bool freezeDelFramesPos(int bone)
   {
      if(skel)if(AnimKeys *keys=findKeys(bone))if(keys.poss.elms()){undos.set("delAll"); anim.freezeDelPos(*skel, bone, -1); return true;}
      return false;
   }
   bool freezeDelFrameOrn(int bone, bool pos)
   {
      if(skel)if(AnimKeys *keys=findKeys(bone))if(AnimKeys.Orn *key=FindOrn(*keys, animTime())){undos.set("del"); anim.freezeDelRot(*skel, bone, keys.orns.index(key), pos); return true;}
      return false;
   }
   bool freezeDelFramesOrn(int bone, bool pos)
   {
      if(skel)if(AnimKeys *keys=findKeys(bone))if(keys.orns.elms()){undos.set("delAll"); anim.freezeDelRot(*skel, bone, -1, pos); return true;}
      return false;
   }

   void delFrame()
   {
      bool changed=false;
      if(op()==OP_ORN   || op()<0)changed|=delFrameOrn  (sel_bone);
      if(op()==OP_ORN2           )changed|=delFrameOrn  (sel_bone)|delFrameOrn(boneParent(sel_bone));
      if(op()==OP_POS   || op()<0)changed|=delFramePos  (sel_bone);
      if(op()==OP_SCALE || op()<0)changed|=delFrameScale(sel_bone);
      if(changed){setAnimSkel(); setOrnTarget(); anim.setRootMatrix(); setChanged();}
   }
   void delFrames(int bone)
   {
      bool changed=false;
      if(op()==OP_ORN   || op()<0)changed|=delFramesOrn  (bone);
      if(op()==OP_ORN2           )changed|=delFramesOrn  (bone)|delFramesOrn(boneParent(bone));
      if(op()==OP_POS   || op()<0)changed|=delFramesPos  (bone);
      if(op()==OP_SCALE || op()<0)changed|=delFramesScale(bone);
      if(changed){setAnimSkel(); setOrnTarget(); anim.setRootMatrix(); setChanged();}
   }
   void freezeDelFrame(bool pos=true)
   {
      bool changed=false;
      if(op()==OP_ORN   || op()<0)changed|=freezeDelFrameOrn  (sel_bone, pos);
      if(op()==OP_ORN2           )changed|=freezeDelFrameOrn  (sel_bone, pos)|freezeDelFrameOrn(boneParent(sel_bone), pos);
      if(op()==OP_POS   || op()<0)changed|=freezeDelFramePos  (sel_bone);
    //if(op()==OP_SCALE || op()<0)changed|=freezeDelFrameScale(sel_bone);
      if(changed){setAnimSkel(); setOrnTarget(); anim.setRootMatrix(); setChanged();}
   }
   void freezeDelFrames(bool pos=true)
   {
      bool changed=false;
      if(op()==OP_ORN   || op()<0)changed|=freezeDelFramesOrn  (sel_bone, pos);
      if(op()==OP_ORN2           )changed|=freezeDelFramesOrn  (sel_bone, pos)|freezeDelFrameOrn(boneParent(sel_bone), pos);
      if(op()==OP_POS   || op()<0)changed|=freezeDelFramesPos  (sel_bone);
    //if(op()==OP_SCALE || op()<0)changed|=freezeDelFramesScale(sel_bone);
      if(changed){setAnimSkel(); setOrnTarget(); anim.setRootMatrix(); setChanged();}
   }
   void delFramesAtEnd()
   {
      if(anim)
      {
         undos.set("delEndKeys");
         if(DelEndKeys(*anim)){setAnimSkel(); setOrnTarget(); setChanged();}
      }
   }
   void reverseFrames()
   {
      if(anim)
      {
         undos.set("reverse");
         if(sel_bone<0                       )anim.reverse();else // reverse entire animation when no bone is selected (instead of just root)
         if(AnimKeys *keys=findKeys(sel_bone)){keys.reverse(anim.length()); anim.setRootMatrix();}
         setAnimSkel(); setOrnTarget(); setChanged();
      }
   }
   void freezeBone()
   {
      if(skel && anim)
      {
         if(!InRange(sel_bone, skel.bones)){Gui.msgBox(S, "No bone selected"); return;}
         undos.set("freezeBone");
         anim.freezeBone(*skel, sel_bone);
         setAnimSkel(); setOrnTarget(); setChanged();
      }
   }
   void playToggle()
   {
      play.toggle();
      preview.toGui();
   }
   void playUpdate(flt multiplier=1)
   {
      bool play=(Kb.b(KB_W) && Kb.ctrlCmd() && !Kb.alt() && (fullscreen ? contains(Gui.kb()) : preview.contains(Gui.kb()))); if(play)T.force_play.push();
      play|=T.force_play(); if(play && Kb.shift())CHS(multiplier);
      play|=T.play();
      if(anim)
      {
         if(flt length=anim.length())
         {
            if(play && !Ms.b(1))_anim_time+=Time.ad()*anim_speed*multiplier;
            if(_anim_time>=length && _anim_time<length+EPS)_anim_time=length;else _anim_time=Frac(_anim_time, length); // allow being at the end
         }else _anim_time=0;
      }
      if(play){setAnimSkel(); setOrnTarget();}
   }
   void setAnimSkel(bool force=false)
   {
      Animation *anim=getVisAnim();
      if(anim && skel)
      {
         if(anim_skel.skeleton()!=skel || anim_skel.bones.elms()!=skel.bones.elms() || anim_skel.slots.elms()!=skel.slots.elms() || force)anim_skel.create(skel);
         skel_anim.create(*skel, *anim);
         flt time=animTime(), time_prev=time-1.0/60/*Time.ad()*/; // always set previous frame to setup correct motion blur velocities when manually changing anim time (use 1/60 to have a constant blur as if for 60 fps)
         // instead of animating root directly, use 'getRootMatrixCumulative' to support correct motion blur when time is at the start to avoid time wrapping/clamping
         Matrix matrix_prev; anim.getRootMatrixCumulative(matrix_prev, time_prev); anim_skel.updateBegin().clear().animateEx(skel_anim, time_prev, true, false, true).updateMatrix(matrix_prev).updateEnd(); // use 'animateEx' to allow editing/previewing exact last keyframes without wrapping to beginning with Frac
         Matrix matrix     ; anim.getRootMatrixCumulative(matrix     , time     ); anim_skel.updateBegin().clear().animateEx(skel_anim, time     , true, false, true).updateMatrix(matrix     ).updateEnd();
         anim_skel.root.orn=anim_skel.matrix(); anim_skel.root.orn.fix(); anim_skel.root.pos=anim_skel.pos(); anim_skel.root.scale=ScaleFactorR(anim_skel.matrix().scale()); // because we don't animate 'root' but the main matrix, we need to setup root based on obtained matrix, this is needed in case we want to access root data (for example adding new keyframes for root positions needs current position)
      }else
      {
         anim_skel.del().updateBegin().updateMatrix().updateEnd();
      }
   }
   void setMeshSkel(bool force=false)
   {
      if(ElmAnim *anim_data=data())if(Elm *skel_elm=Proj.findElm(anim_data.skel_id))if(ElmSkel *skel_data=skel_elm.skelData())if(skel_data.mesh_id.valid()) // get mesh from anim->skel->mesh
      {
         if(mesh_id!=skel_data.mesh_id
         || skel_id!=skel_elm .id
         || force)
         {
            mesh_id=skel_data.mesh_id;
            skel_id=skel_elm .id;
         #if 0 // load Game mesh (faster but uses "body skel")
            mesh=Proj.gamePath(skel_data.mesh_id);
            skel=mesh->skeleton();
         #else // load Edit mesh (slower but uses "original mesh skel")
            skel=&T.skel_data; T.skel_data.load(Proj.gamePath(skel_id));
            mesh=&  mesh_data; Load(mesh_data, Proj.editPath(mesh_id), Proj.game_path); RemovePartsAndLods(mesh_data); mesh_data.transform(skel_data.transform());
            mesh_data.skeleton(skel).setTanBinDbl().setRender(false);
         #endif
            slot_meshes.clear();
         }
         setAnimSkel(true);
         return;
      }
      anim_skel.del();
      mesh_data.del(); mesh=null; mesh_id.zero();
      skel_data.del(); skel=null; skel_id.zero();
   }
   void toGui()
   {
      REPAO(     props).toGui();
      REPAO(root_props).toGui();
                preview.toGui();
      ElmAnim *data=T.data();
      loop           .set(data && data.loop  (), QUIET);
      linear         .set(data && data.linear(), QUIET);
      root_from_body .set(data && FlagOn(data.flag, ElmAnim.ROOT_FROM_BODY ), QUIET);
      root_del_pos_x .set(data && FlagOn(data.flag, ElmAnim.ROOT_DEL_POS_X ), QUIET);
      root_del_pos_y .set(data && FlagOn(data.flag, ElmAnim.ROOT_DEL_POS_Y ), QUIET);
      root_del_pos_z .set(data && FlagOn(data.flag, ElmAnim.ROOT_DEL_POS_Z ), QUIET);
      root_del_rot_x .set(data && FlagOn(data.flag, ElmAnim.ROOT_DEL_ROT_X ), QUIET);
      root_del_rot_y .set(data && FlagOn(data.flag, ElmAnim.ROOT_DEL_ROT_Y ), QUIET);
      root_del_rot_z .set(data && FlagOn(data.flag, ElmAnim.ROOT_DEL_ROT_Z ), QUIET);
      root_smooth_rot.set(data && FlagOn(data.flag, ElmAnim.ROOT_SMOOTH_ROT), QUIET);
      root_smooth_pos.set(data && FlagOn(data.flag, ElmAnim.ROOT_SMOOTH_POS), QUIET);
      root_set_move  .set(data && data.rootMove(), QUIET);
      root_set_rot   .set(data && data.rootRot (), QUIET);
   }
   void applySpeed()
   {
      if(anim && !Equal(anim_speed, 1) && !Equal(anim_speed, 0))if(ElmAnim *data=T.data())
      {
         undos.set("speed");

         // adjust speed file param
         Mems<FileParams> file_params=FileParams.Decode(data.src_file);
         if(FileParams *fps=file_params.data())
         {
            TextParam &speed=fps.getParam("speed");
            flt set_speed=anim_speed; if(flt cur_speed=speed.asFlt())set_speed*=cur_speed;
            if(Equal(set_speed, 1))fps.params.removeData(&speed, true);else speed.setValue(set_speed);
            data.setSrcFile(FileParams.Encode(file_params));
         }
         {
            FileParams fps=data.imported_file_params;
            TextParam &speed=fps.getParam("speed");
            flt set_speed=anim_speed; if(flt cur_speed=speed.asFlt())set_speed*=cur_speed;
            if(Equal(set_speed, 1))fps.params.removeData(&speed, true);else speed.setValue(set_speed);
            data.imported_file_params=fps.encode();
         }

         anim.length(anim.length()/anim_speed, true);
         setChanged();
         anim_speed=1;
         toGui();
      }
   }
   void  setEvent(  AnimEvent &e, int i)C {if(anim && InRange(i, anim.events))e=anim.events[i];}
   int  findEvent(C AnimEvent &e       )C {if(anim)REPA(anim.events)if(e==anim.events[i])return i; return -1;}
   void moveEvent(int event, flt time)
   {
      if(anim && InRange(event, anim.events))
      {
         undos.set("eventMove");
         anim.events[event].time=time;
         AnimEvent s, ps, l, pl;
         setEvent(s, track.event_sel); setEvent(ps, preview.track.event_sel);
         setEvent(l, track.event_lit); setEvent(pl, preview.track.event_lit);
         anim.sortEvents();
         track.event_sel=findEvent(s); preview.track.event_sel=findEvent(ps);
         track.event_lit=findEvent(l); preview.track.event_lit=findEvent(pl);
         setChanged();
      }
   }
   void newEvent(flt time)
   {
      if(anim)
      {
         undos.set("event"); // keep the same as 'renameEvent' because they're linked
         Str  name=RenameEvent.textline().is() ? RenameEvent.textline() : S+"Event"; // reuse last name if available
         bool edit_name=!Kb.ctrlCmd();

         // if there's already an event with same name at the same time, then don't add new, due to accidental clicks
         if(!(edit_name && !RenameEvent.visibleOnActiveDesktop())) // always add if we're going to show rename window as long as it's hidden
            REPA(anim.events)
         {
          C AnimEvent &event=anim.events[i];
            if(Equal(event.name, name) && Abs(event.time-time)<=TimeEps())return;
         }

         AnimEvent event=anim.events.New().set(name, time);
         anim.sortEvents();
         setChanged();
         if(edit_name)RenameEvent.activate(findEvent(event)); // activate window for renaming created event
      }
   }
   void delEvent(int index)
   {
      if(anim && InRange(index, anim.events))
      {
         undos.set("eventDel");
         anim.events.remove(index, true);
         setChanged();
                 track.removedEvent(index);
         preview.track.removedEvent(index);
      }
   }
   void renameEvent(int index, C Str &old_name, C Str &new_name)
   {
      if(anim)FREPA(anim.events)if(old_name==anim.events[i].name)if(!index--)
      {
         undos.set("event"); // keep the same as 'newEvent' because they're linked
         Set(anim.events[i].name, new_name);
         anim.sortEvents();
         setChanged();
         break;
      }
   }
   void flush()
   {
      if(elm && changed)
      {
         if(ElmAnim *data=elm.animData()){data.newVer(); if(anim)data.from(*anim);} // modify just before saving/sending in case we've received data from server after edit
         if(anim){Save(*anim, Proj.gamePath(elm.id)); Proj.savedGame(*elm);}
         Server.setElmLong(elm.id);
      }
      changed=false;
   }
   void setChanged(bool file=true)
   {
      if(elm)
      {
         changed=true;
         if(ElmAnim *data=elm.animData())
         {
            data.newVer();
            if(anim)data.from(*anim);
            if(file)data.file_time.getUTC();
            optimize_anim .refresh();
            scale_pos_keys.refresh();
         }
      }
   }
   void validateFullscreen()
   {
      Mode   .tabAvailable(MODE_ANIM, fullscreen && elm);
      preview.visible     (          !fullscreen && elm);
   }
   void toggleFullscreen()
   {
      fullscreen^=1;
      validateFullscreen();
      activate(elm);
      preview.kbSet();
      toGui();
   }
   void set(Elm *elm)
   {
      if(elm && elm.type!=ELM_ANIM)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();

         if(skel)
         {
            sel_bone_name=(InRange(sel_bone, skel.bones) ? skel.bones[sel_bone].name : null);
         }

         T.elm   =elm;
         T.elm_id=(elm ? elm.id : UIDZero);
         if(elm)anim=Animations(Proj.gamePath(elm.id));else anim=null;
         toGui();
         Proj.refresh(false, false);
         validateFullscreen();
         RenameEvent.hide();

         optimize_anim .refresh();
         scale_pos_keys.refresh();
         setMeshSkel();
         selBone(skel ? skel.findBoneI(sel_bone_name) : -1);
         setOrnTarget();
         optimize_anim   .hide();
         scale_pos_keys  .hide();
         time_range_speed.hide();
         Gui.closeMsgBox(transform_obj_dialog_id);
         preview.moveToTop();
      }
   }
   void focus()
   {
      if(T.elm){if(fullscreen){Mode.set(MODE_ANIM); HideBig();}else preview.activate();}
   }
   void activate(Elm *elm)
   {
      set(elm); focus();
   }
   void toggle(Elm *elm)
   {
      if(elm==T.elm && (fullscreen ? selected() : true))elm=null;
      if(fullscreen || preview.maximized())activate(elm);else set(elm); // if editor is in fullscreen mode, or the window is maximized then activate
   }
   void elmChanging(C UID &elm_id)
   {
      if(elm && elm.id==elm_id)
      {
         undos.set(null, true);
      }
   }
   void elmChanged(C UID &elm_id)
   {
      if(elm && elm.id==elm_id)
      {
         setMeshSkel();
         setOrnTarget();
         toGui();
      }
   }
   void erasing(C UID &elm_id) {if(elm && elm.id==elm_id)set(null);}
   void setTarget(C Str &obj_id)
   {
      if(anim)if(ElmAnim *anim_data=data())
      {
         if(obj_id.is())
         {
            if(Elm *obj=Proj.findElm(obj_id))if(ElmObj *obj_data=obj.objData())
            {
               bool has_skel=false;
               if(Elm *mesh=Proj.findElm( obj_data.mesh_id))if(ElmMesh *mesh_data=mesh.meshData())
               if(Elm *skel=Proj.findElm(mesh_data.skel_id))if(ElmSkel *skel_data=skel.skelData())
               {
                  has_skel=true;
                  if(skel.id!=anim_data.skel_id)
                  {
                     undos.set("skel");

                     /*if(0) // don't transform, because for example if original mesh was super small, then it had to be scaled up, making the transform very big, and when transforming to new transform the scale could get big, so just accept the current transform, assume that user has scaled the orignal mesh/skel/anim to match the new mesh/skel/anim
                     {
                        Elm      *old_skel_elm=Proj.findElm(anim_data.skel_id, ELM_SKEL);
                      C Skeleton *old_skel=(old_skel_elm ? Skeletons(Proj.gamePath(*old_skel_elm)) : null),
                                 *new_skel=                Skeletons(Proj.gamePath(     skel.id ));

                        anim.transform(GetTransform(anim_data.transform(), skel_data.transform()), old_skel ? *old_skel : *new_skel); // transform from current to target, if 'old_skel' is not available then try using 'new_skel'
                     }*/
                     if(RenameAnimBonesOnSkelChange)if(Skeleton *new_skel=Skeletons(Proj.gamePath(skel.id)))
                     {
                        if(ObjEdit.skel_elm && ObjEdit.skel_elm.id==skel.id)ObjEdit.flushMeshSkel(); // !! if this skeleton is currently edited, then we have to flush it, for example, it could have been empty at the start and we've just added bones, so 'new_skel' is empty and bones are only in 'ObjEdit.mesh_skel_temp' memory, and before changing 'anim_data.skel_id' !!
                        anim.setBoneNameTypeIndexesFromSkeleton(*new_skel);
                     }

                     anim_data.skel_id  =skel.id; // !! set after 'flushMeshSkel' !!
                     anim_data.skel_time.getUTC();
                     anim_data.transform=skel_data.transform;
                     setChanged(); setMeshSkel();
                     toGui(); Proj.refresh(false, false);
                  }
               }
               if(!has_skel)Gui.msgBox(S, "Can't set target object because it doesn't have a mesh skeleton.");
            }
         }else
         {
            undos.set("skel");
            anim_data.skel_id.zero();
            anim_data.skel_time.getUTC();
            setChanged(); setMeshSkel();
            toGui(); Proj.refresh(false, false);
         }
      }
   }
   void drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos)
   {
      bool preview_contains=preview.contains(obj);
      if(contains(obj) || preview_contains)
         REPA(elms)
            if(Elm *obj=Proj.findElm(elms[i], ELM_OBJ))
      {
         if(preview_contains && skel && InRange(preview.lit_slot, skel.slots))
         {
            SlotMesh.Set(slot_meshes, skel.slots[preview.lit_slot].name, Proj.gamePath(Proj.objToMesh(obj)));
         }else setTarget(obj.id.asHex());
         elms.remove(i, true);
         break;
      }
   }

private:
   flt _anim_time=0;
}
AnimEditor AnimEdit;
/******************************************************************************/

