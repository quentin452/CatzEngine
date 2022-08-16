/******************************************************************************

   Android platform application life cycle:
      -constructors for global objects are called
      -'android_main' is     called       (with full loop)
      -'android_main' can be called again (with full loop)
      -..
      -if OS decides to kill the app, then probably destructors will be called (but it is unknown)

   When using WORK_IN_BACKGROUND then 'android_main' with full loop will continue to run even when app is closed/minimized.
   In that case when trying to remove the app from recent apps list (for example by "close all" or "swipe" in the apps list)
      then the loop will be stopped and 'android_main' will return, HOWEVER 'BackgroundThread' will continue to run.
   After that, 'android_main' may be called again if the app is started again.

   JNI object life exists as long as we're in native (C++) mode, it will be automatically deleted once we're back to Java.
      Therefore objects that want to be stored globally across multiple threads, must use 'NewGlobalRef' and later 'DeleteGlobalRef'.
      'jfieldID' and 'jmethodID' should NOT be passed to 'NewGlobalRef', 'DeleteGlobalRef' or 'DeleteLocalRef'.

   https://developer.android.com/training/articles/perf-jni.html

   JNI Sample Usage documentation link:
      http://android.wooyd.org/JNIExample/

   'KeyCharacterMapGet' does not convert meta to accents, what we would need are accented characters to be used with 'getDeadChar'.
      However NDK input does not provide COMBINING_ACCENT and COMBINING_ACCENT_MASK for key codes.
      I still need to use 'KeyCharacterMap' instead of converting buttons to characters, because on Swiftkey Android 2.3 pressing '(' results in 'k' character, and 'KeyCharacterMap' fixes that.

   Type Signature Java Type
   Z              boolean
   B              byte
   C              char
   S              short
   I              int
   J              long
   F              float
   D              double
   L*;            "*" class
   [*             *[] array
   (params)ret    method

   For example, the Java method:     long f(int n, String s, int[] arr);
   has the following type signature: (ILjava/lang/String;[I)J
/******************************************************************************/
#include "stdafx.h"
#if ANDROID

#include "../../../../ThirdPartyLibs/begin.h"
#include "../../../../ThirdPartyLibs/play-core-native-sdk/include/asset_pack.h"
#include "../../../../ThirdPartyLibs/end.h"

ASSERT(SIZE(Long)>=SIZE(Ptr)); // some pointers are processed using Long's on Java
namespace EE{
/******************************************************************************/
#if DEBUG
   #define LOG(x) LogN(x)
#else
   #define LOG(x) if(LogInit)LogN(x)
#endif
   #define LOG2(x)
/******************************************************************************/
enum ROTATION
{
   ROTATION_0  ,
   ROTATION_90 ,
   ROTATION_180,
   ROTATION_270,
};
/******************************************************************************/
JNI Jni(null);

static JavaVM        *JVM;
       jobject        Activity;
       JClass         ActivityClass, ClipboardManagerClass, KeyCharacterMapClass;
       JObject        DefaultDisplay, ClipboardManager, LocationManager, KeyCharacterMap,
                      GPS_PROVIDER, NETWORK_PROVIDER, EsenthelLocationListener[2];
       JMethodID      getRotation,
                      KeyCharacterMapLoad, KeyCharacterMapGet,
                      getLastKnownLocation, getLatitude, getLongitude, getAltitude, getAccuracy, getSpeed, getTime, requestLocationUpdates, removeUpdates, vibrate;
       Int            AndroidSDK;
       AAssetManager *AssetManager;
       android_app   *AndroidApp;
       Str8           AndroidPackageName;
       Str            AndroidAppPath, AndroidAppDataPath, AndroidAppDataPublicPath, AndroidAppCachePath, AndroidPublicPath, AndroidSDCardPath;
static KB_KEY         KeyMap[256];
static Byte           JoyMap[256];
static Bool           Initialized, // !! This may be set to true when app is restarted (without previous crashing) !!
                      KeyboardLoaded, PossibleTap;
static Int            KeyboardDeviceID;
static Dbl            PossibleTapTime;
static VecI2          LastMousePos;
static Thread         BackgroundThread;
/******************************************************************************/
static Bool BackgroundFunc(Thread &thread)
{
   LOG("BackgroundFunc");
   ThreadMayUseGPUData(); // here we call Update functions which normally assume to be called on the main thread, so make sure GPU can be used just like on the main thread
   Jni.attach();
   LOG("BackgroundFunc Loop");
   for(;;) // do a manual loop to avoid the overhead of 'ThreadMayUseGPUData' and 'ThreadFunc' (which is needed only if we may want to pause the thread, which we won't)
   {
      if(App.background_wait)thread._resume.wait(App.background_wait); // wait requested time, Warning: here we reuse its 'SyncEvent' because it's used only for pausing/resuming
      if(App._close) // first check if we want to close manually
      {
         App.del(); // manually call shut down
         ExitNow(); // do complete reset (including state of global variables) by killing the process
      }
      if(thread.wantStop())break; // check after waiting and before updating
      App.update();
   }
   Jni.del();
   LOG("BackgroundFunc End");
   return false;
}
/******************************************************************************/
void JNI::clear()
{
   _=null;
   attached=false;
}
void JNI::del()
{
   if(_ && attached)JVM->DetachCurrentThread();
   clear();
}
void JNI::attach()
{
   if(!_)
   {
      if(!JVM)Exit("JVM is null");
      JVM->GetEnv((Ptr*)&_, JNI_VERSION_1_6);
      if(!_)
      {
         JVM->AttachCurrentThread(&_, null);
         if(_)attached=true;
      }
      if(!_)Exit("Can't attach thread to Java Virtual Machine");
   }
}
jmethodID JNI::      func(jclass clazz, const char* name, const char* sig)C {jmethodID f=_->GetMethodID      (clazz, name, sig); if(_->ExceptionCheck()){_->ExceptionClear(); f=null;} return f;}
jmethodID JNI::staticFunc(jclass clazz, const char* name, const char* sig)C {jmethodID f=_->GetStaticMethodID(clazz, name, sig); if(_->ExceptionCheck()){_->ExceptionClear(); f=null;} return f;}

JObject& JObject::clear     () {_=null; _global=false; return T;}
JObject& JObject::del       () {if(_ && _jni){if(_global)_jni->DeleteGlobalRef(_);else _jni->DeleteLocalRef(_);} _=null; _global=false; return T;}
JObject& JObject::makeGlobal() {if(!_global && _jni && _){jobject j=_jni->NewGlobalRef(_); del(); _=j; _global=true;} return T;}
JObject& JObject::operator= (jobject j) {del(); _=j; _global=false; return T;}

JClass& JClass::operator=(CChar8 *name) {del(); _=_jni->FindClass     (name); _global=false; return T;}
JClass& JClass::operator=(jobject obj ) {del(); _=_jni->GetObjectClass(obj ); _global=false; return T;}
JClass& JClass::operator=(jclass  cls ) {del(); _=                     cls  ; _global=false; return T;}

JClass::JClass(          CChar8 *name) : JObject(     name ? Jni->FindClass     (name) : null) {}
JClass::JClass(          jobject obj ) : JObject(            Jni->GetObjectClass(obj )       ) {}
JClass::JClass(          jclass  cls ) : JObject(                                cls         ) {}
JClass::JClass(JNI &jni, CChar8 *name) : JObject(jni, name ? jni->FindClass     (name) : null) {}
JClass::JClass(JNI &jni, jobject obj ) : JObject(jni,        jni->GetObjectClass(obj )       ) {}
JClass::JClass(JNI &jni, jclass  cls ) : JObject(jni,                            cls         ) {}

JString& JString::operator=(CChar8 *t) {T=_jni->NewStringUTF(        t              ); return T;}
JString& JString::operator=(CChar  *t) {T=_jni->NewString   ((jchar*)t  ,  Length(t)); return T;}
JString& JString::operator=(C Str8 &s) {T=_jni->NewStringUTF(        s()            ); return T;}
JString& JString::operator=(C Str  &s) {T=_jni->NewString   ((jchar*)s(), s.length()); return T;}

Int     JObjectArray::elms      (     )C {return _jni->GetArrayLength       (T   );}
jobject JObjectArray::operator[](Int i)C {return _jni->GetObjectArrayElement(T, i);}

     JObjectArray::JObjectArray(JNI &jni, int elms) : JObject(jni, jni->NewObjectArray(elms, jni->FindClass("java/lang/String"), NULL)) {}
void JObjectArray::set(Int i, CChar8 *t) {if(_ && _jni)_jni->SetObjectArrayElement(T, i, _jni->NewStringUTF(t));}
/******************************************************************************/
void Overlay(C Str &text)
{
#if ANDROID
   JNI jni;
   if(jni && ActivityClass)
   if(JMethodID overlay=jni.staticFunc(ActivityClass, "overlay", "(Ljava/lang/String;)V"))
      if(JString jtext=JString(jni, text))
         jni->CallStaticVoidMethod(ActivityClass, overlay, jtext());
#endif
}
/******************************************************************************/
static void UpdateOrientation()
{
 //switch(AConfiguration_getOrientation(AndroidApp->config)) this appears to have weird enum values: ACONFIGURATION_ORIENTATION_ANY, ACONFIGURATION_ORIENTATION_PORT, ACONFIGURATION_ORIENTATION_LAND, ACONFIGURATION_ORIENTATION_SQUARE
   if(Jni && getRotation)switch(Jni->CallIntMethod(DefaultDisplay, getRotation))
   {
      case ROTATION_0  : App._orientation=DIR_UP   ; break;
      case ROTATION_90 : App._orientation=DIR_LEFT ; break;
      case ROTATION_180: App._orientation=DIR_DOWN ; break;
      case ROTATION_270: App._orientation=DIR_RIGHT; break;
   }
}
static void UpdateSize()
{
   if(App.window())
   {
      VecI2 res(ANativeWindow_getWidth (App.window()),
                ANativeWindow_getHeight(App.window()));
      if(D.res()!=res && res.x>0 && res.y>0)
      {
         LOG(S+"Resize: "+res.x+", "+res.y);
         Renderer._main.forceInfo(res.x, res.y, 1, Renderer._main.type(), Renderer._main.mode(), Renderer._main.samples()); // '_main_ds' will be set in 'rtCreate'
         D.modeSet(res.x, res.y);
      }
   }
}
/******************************************************************************/
static int32_t InputCallback(android_app *app, AInputEvent *event)
{
   LOG2("InputCallback");
   Int device=AInputEvent_getDeviceId(event);
   Int source=AInputEvent_getSource  (event);
   switch(    AInputEvent_getType    (event))
   {
      case AINPUT_EVENT_TYPE_MOTION:
      {
         Int  action      =AMotionEvent_getAction(event),
              action_type = (action&AMOTION_EVENT_ACTION_MASK),
              action_index=((action&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)>>AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
         Bool stylus      =FlagOn(source, AINPUT_SOURCE_STYLUS & ~AINPUT_SOURCE_CLASS_POINTER); // disable 'AINPUT_SOURCE_CLASS_POINTER' because (AINPUT_SOURCE_TOUCHSCREEN, AINPUT_SOURCE_MOUSE, AINPUT_SOURCE_STYLUS) have it
         if(source&((AINPUT_SOURCE_DPAD|AINPUT_SOURCE_GAMEPAD|AINPUT_SOURCE_JOYSTICK) & ~AINPUT_SOURCE_CLASS_BUTTON)) // disable 'AINPUT_SOURCE_CLASS_BUTTON' because AINPUT_SOURCE_KEYBOARD has it
         {
            if(action_type==AMOTION_EVENT_ACTION_MOVE)
               if(Joypad *joypad=Joypads.find(device))
            {
                                                joypad->dir     .set( AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_X, action_index),
                                                                     -AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_Y, action_index));
                                                joypad->dir_a[0].set( AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_X    , action_index),
                                                                     -AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Y    , action_index));
                                                joypad->dir_a[1].set( AMotionEvent_getAxisValue(event, joypad->_axis_stick_r_x , action_index),  // for performance set without checking for valid axis, in worst case, 0 is reported
                                                                     -AMotionEvent_getAxisValue(event, joypad->_axis_stick_r_y , action_index)); // for performance set without checking for valid axis, in worst case, 0 is reported
               if(joypad->_axis_trigger_l!=0xFF)joypad->setTrigger(0, AMotionEvent_getAxisValue(event, joypad->_axis_trigger_l , action_index) * joypad->_axis_trigger_mad[0].x + joypad->_axis_trigger_mad[0].y); // update only if have axis, to avoid conflicts with digital-bit-trigger buttons
               if(joypad->_axis_trigger_r!=0xFF)joypad->setTrigger(1, AMotionEvent_getAxisValue(event, joypad->_axis_trigger_r , action_index) * joypad->_axis_trigger_mad[1].x + joypad->_axis_trigger_mad[1].y); // update only if have axis, to avoid conflicts with digital-bit-trigger buttons
            #if DEBUG && 0
               REP(256){Flt v=AMotionEvent_getAxisValue(event, i, action_index); if(v!=0)LogN(S+"Joy Axis: "+i+", Value: "+v);}
            #endif
               joypad->setDiri(Round(joypad->dir.x), Round(joypad->dir.y));
            }
         }else
         if((source&(AINPUT_SOURCE_MOUSE & ~AINPUT_SOURCE_CLASS_POINTER)) && !stylus) // disable 'AINPUT_SOURCE_CLASS_POINTER' because (AINPUT_SOURCE_TOUCHSCREEN, AINPUT_SOURCE_MOUSE, AINPUT_SOURCE_STYLUS) have it, check for 'stylus' because on "Samsung Galaxy Note 2" stylus input generates both "AINPUT_SOURCE_STYLUS|AINPUT_SOURCE_MOUSE" at the same time
         {
            if(action_type==AMOTION_EVENT_ACTION_SCROLL)
            {
               Ms._wheel.x+=AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HSCROLL, action_index);
               Ms._wheel.y+=AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_VSCROLL, action_index);
            }

            // get button states
            Int button_state=AMotionEvent_getButtonState(event);
            if(action_type==AMOTION_EVENT_ACTION_DOWN && !button_state){PossibleTap=true; PossibleTapTime=Time.appTime();}else // 'getButtonState' does not detect tapping on the touchpad, so we need to detect it according to 'AMOTION_EVENT_ACTION_DOWN', also proceed only if no buttons are pressed in case this event is triggered by secondary mouse button
            if(PossibleTap && (button_state || (LastMousePos-Ms.desktopPos()).abs().max()>=6))PossibleTap=false; // if we've pressed a button or moved away too much then it's definitely not a tap
            if(action_type==AMOTION_EVENT_ACTION_UP   &&  PossibleTap ){PossibleTap=false; if(Time.appTime()<=PossibleTapTime+DoubleClickTime+Time.ad())Ms.push(0);} // this is a tap so push the button and it will be released line below because 'button_state' is 0
            REPA(Ms._button)if(FlagOn(button_state, 1<<i)!=Ms.b(i))if(Ms.b(i))Ms.release(i);else Ms.push(i);

            // get scrolling and cursor position
            if(action_type!=AMOTION_EVENT_ACTION_UP // this can happen on release of TouchPad scroll, where the position is still at the dragged position
            && AMotionEvent_getPointerCount(event)>=1)
            {
               VecI2 pos(Round(AMotionEvent_getRawX(event, 0)),
                         Round(AMotionEvent_getRawY(event, 0)));
               if(action_type==AMOTION_EVENT_ACTION_MOVE // if we're dragging
               && !button_state) // none of them are pressed
               { // this is TouchPad scroll (with 2 fingers on the TouchPad)
                  const Flt mul=16.0f/D.screen().min(); // trigger 16 full mouse wheel events when moving mouse from one side to another (this value was set to match results on a Windows laptop, use 'min' or 'max' to get the same results even when device got rotated and that would cause resolution to change)
                  Ms._wheel.x-=(pos.x-LastMousePos.x)*mul;
                  Ms._wheel.y+=(pos.y-LastMousePos.y)*mul;
               }else
               {
                  Ms.  _delta_pixeli_clp+=pos-Ms.desktopPos(); // calc based on desktop position and not window position, += because this can be called several times per frame
                  Ms._desktop_pixeli     =pos;
                  Ms. _window_pixeli.x   =Round(AMotionEvent_getX(event, 0));
                  Ms. _window_pixeli.y   =Round(AMotionEvent_getY(event, 0));
               }
               LastMousePos=pos;
            }
            Ms._hardware=true; // TODO: this could move to some device added/removed callback
         }else
         {
            switch(action_type)
            {
               case AMOTION_EVENT_ACTION_HOVER_ENTER: // touch is not pressed but appeared
               case AMOTION_EVENT_ACTION_DOWN       : // touch was    pressed (1st pointer)
               {
                  REP(AMotionEvent_getPointerCount(event))
                  {
                     CPtr   pid   =(CPtr)AMotionEvent_getPointerId(event, i);
                     Vec2   pixel (AMotionEvent_getX(event, i), AMotionEvent_getY(event, i)),
                            pos   =D.windowPixelToScreen(pixel);
                     VecI2  pixeli=Round(pixel);
                     Touch *touch =FindTouchByHandle(pid);
                     if(   !touch)touch=&Touches.New().init(pixeli, pos, pid, stylus);else
                     {
                        touch->_remove=false; // disable 'remove' in case it was enabled (for example the same touch was released in same/previous frame)
                        if(action_type!=AMOTION_EVENT_ACTION_HOVER_ENTER)touch->reinit(pixeli, pos); // re-initialize for push (don't do this for hover because it can be called the same frame that release is called, and for release we want to keep the original values)
                     }
                     if(action_type!=AMOTION_EVENT_ACTION_HOVER_ENTER)touch->_state=BS_ON|BS_PUSHED;
                  }
               }break;

               case AMOTION_EVENT_ACTION_POINTER_DOWN: // touch was pressed (secondary pointer)
               {
                  CPtr   pid   =(CPtr)AMotionEvent_getPointerId(event, action_index);
                  Vec2   pixel (AMotionEvent_getX(event, action_index), AMotionEvent_getY(event, action_index)),
                         pos   =D.windowPixelToScreen(pixel);
                  VecI2  pixeli=Round(pixel);
                  Touch *touch =FindTouchByHandle(pid);
                  if(   !touch)touch=&Touches.New().init(pixeli, pos, pid, stylus);else
                  {
                     // re-initialize for push
                     touch->_remove=false; // disable 'remove' in case it was enabled (for example the same touch was released in same/previous frame)
                     touch-> reinit(pixeli, pos);
                  }
                  touch->_state=BS_ON|BS_PUSHED;
               }break;

               case AMOTION_EVENT_ACTION_MOVE:       // touch is     pressed and was moved
               case AMOTION_EVENT_ACTION_HOVER_MOVE: // touch is not pressed and was moved
               {
                  REP(AMotionEvent_getPointerCount(event))
                  {
                     CPtr   pid   =(CPtr)AMotionEvent_getPointerId(event, i);
                     Vec2   pixel (AMotionEvent_getX(event, i), AMotionEvent_getY(event, i)),
                            pos   =D.windowPixelToScreen(pixel);
                     VecI2  pixeli=Round(pixel);
                     Touch *touch =FindTouchByHandle(pid);
                     if(   !touch)touch=&Touches.New().init(pixeli, pos, pid, stylus);else
                     {  // update
                        touch->_delta_pixeli_clp+=pixeli-touch->_pixeli;
                        touch->_pixeli           =pixeli;
                        touch->_pos              =pos;
                        if(!touch->_state)touch->_gui_obj=Gui.objAtPos(touch->pos()); // when hovering then also update gui object (check for 'state' because hover can be called the same frame that release is called, and for release we want to keep the original values)
                     }
                  }
               }break;

               case AMOTION_EVENT_ACTION_UP:
               case AMOTION_EVENT_ACTION_CANCEL: // release all touches
               {
                  REPA(Touches)
                  {
                     Touch &touch=Touches[i];
                     touch._remove=true;
                     if(touch._state&BS_ON) // check for state in case it was manually eaten
                     {
                        touch._state|= BS_RELEASED;
                        touch._state&=~BS_ON;
                     }
                  }
               }break;

               case AMOTION_EVENT_ACTION_POINTER_UP: // release one touch
               {
                  CPtr pid=(CPtr)AMotionEvent_getPointerId(event, action_index);
                  if(Touch *touch=FindTouchByHandle(pid))
                  {
                     Vec2  pixel (AMotionEvent_getX(event, action_index), AMotionEvent_getY(event, action_index));
                     VecI2 pixeli=Round(pixel);
                     touch->_delta_pixeli_clp+=pixeli-touch->_pixeli;
                     touch->_pixeli           =pixeli;
                     touch->_pos              =D.windowPixelToScreen(pixel);
                     touch->_remove           =true;
                     if(touch->_state&BS_ON) // check for state in case it was manually eaten
                     {
                        touch->_state|= BS_RELEASED;
                        touch->_state&=~BS_ON;
                     }
                  }
               }break;

               case AMOTION_EVENT_ACTION_HOVER_EXIT: // hover exited screen
               {
                  REP(AMotionEvent_getPointerCount(event))
                  {
                     CPtr pid=(CPtr)AMotionEvent_getPointerId(event, i);
                     if(Touch *touch=FindTouchByHandle(pid))
                     {
                        if(touch->on()) // if was pressed then release it (so application can process release)
                        {
                           touch->_remove= true;
                           touch->_state|= BS_RELEASED;
                           touch->_state&=~BS_ON;
                        }else // if not pressed then just remove it
                        {
                           Touches.removeData(touch, true);
                        }
                     }
                  }
               }break;
            }
         }
#if 0
   Int pc=AMotionEvent_getPointerCount(event);
   Str o=S+"event:"+TextHex((UInt)source, -1, 0, true)+", action:"+Int(action_type)+", action_index:"+Int(action_index)+", ptr_cnt:"+pc+", ptr_id:";
   FREP(pc)o+=S+CPtr(AMotionEvent_getPointerId(event, i))+" | ";
   o+=S+"Touches:"+Touches.elms()+" "; FREPA(Touches)o+=S+'('+Touches[i]._state+", "+Touches[i].pos()+')';
   LogN(o);
#endif
      }return 1;

      case AINPUT_EVENT_TYPE_KEY:
      {
         Int code=AKeyEvent_getKeyCode(event),
           action=AKeyEvent_getAction (event);

         if(source&((AINPUT_SOURCE_DPAD|AINPUT_SOURCE_GAMEPAD|AINPUT_SOURCE_JOYSTICK) & ~AINPUT_SOURCE_CLASS_BUTTON)) // disable 'AINPUT_SOURCE_CLASS_BUTTON' because AINPUT_SOURCE_KEYBOARD has it
         {
            if(Joypad *joypad=Joypads.find(device))
            {
               Byte button;
               Int scan_code=AKeyEvent_getScanCode(event)-304; if(InRange(scan_code, joypad->_remap))
               {
                  button=joypad->_remap[scan_code]; if(button!=255)goto have_button;
               }
               if(InRange(code, JoyMap))
               {
                  button=JoyMap[code]; if(button!=255)
                  {
                  have_button:
                     switch(action)
                     {
                        case AKEY_EVENT_ACTION_DOWN    : joypad->push   (button); break;
                        case AKEY_EVENT_ACTION_UP      : joypad->release(button); break;
                        case AKEY_EVENT_ACTION_MULTIPLE: joypad->push   (button); joypad->release(button); break;
                     }
                  }
               }
            }
            return 1;
         }

         Int    meta =AKeyEvent_getMetaState(event);
         Bool   ctrl =FlagOn(meta, (Int)AMETA_CTRL_ON     ),
                shift=FlagOn(meta, (Int)AMETA_SHIFT_ON    ),
                alt  =FlagOn(meta, (Int)AMETA_ALT_ON      ),
                caps =FlagOn(meta, (Int)AMETA_CAPS_LOCK_ON);
         KB_KEY key  =(InRange(code, KeyMap) ? KeyMap[code] : KB_NONE);
         /*if(shift && !ctrl && !alt && !Kb.anyWin())
         {
            if(key==KB_BACK ){key=KB_DEL; Kb._disable_shift=true;}else // Shift+Back  = Del (this is     universal behaviour on Android platform)
            if(key==KB_ENTER){key=KB_INS; Kb._disable_shift=true;}     // Shift+Enter = Ins (this is non-universal behaviour on Android platform)
         }*/

//LogN(S+"dev:"+device+", code:"+code+", meta:"+meta+", key:"+key+", action:"+action);
//if(!key)LogN(S+"key:"+code);

         // check for characters here still because 'dispatchKeyEvent' does not catch keys from hardware keyboards (like tablet with attachable keyboard)
         Char chr='\0';
         if(KeySource!=KEY_JAVA)
         {
            // get 'KeyCharacterMap' for this device
            if(!KeyboardLoaded || KeyboardDeviceID!=device)
            {
               KeyboardLoaded  =true;
               KeyboardDeviceID=device;
               if(KeyCharacterMapGet)
               {
                  KeyCharacterMap=Jni->CallStaticObjectMethod(KeyCharacterMapClass, KeyCharacterMapLoad, jint(KeyboardDeviceID));
                  if(Jni->ExceptionCheck()){KeyCharacterMap.clear(); Jni->ExceptionClear();}else KeyCharacterMap.makeGlobal();
               }
            }

            if(KeyCharacterMap)
            {
               FlagDisable(meta, Int(AMETA_CTRL_ON|AMETA_CTRL_LEFT_ON|AMETA_CTRL_RIGHT_ON)); // disable CTRL in meta because if it would be present then 'chr' would be zero, we can't remove LALT yet because Swiftkey Android 2.3 relies on it
               chr=Jni->CallIntMethod(KeyCharacterMap, KeyCharacterMapGet, jint(code), jint(meta));
               if(!chr && (meta&AMETA_ALT_LEFT_ON))
               {
                  FlagDisable(meta, (Int)AMETA_ALT_LEFT_ON);
                  FlagSet(meta, (Int)AMETA_ALT_ON, FlagOn(meta, (Int)AMETA_ALT_RIGHT_ON)); // setup correct ALT mask
                  chr=Jni->CallIntMethod(KeyCharacterMap, KeyCharacterMapGet, jint(code), jint(meta));
               }
            }else
            if(!Kb.b(KB_RALT))chr=Kb.keyChar(key, shift, caps);

            if(chr)KeySource=KEY_CPP; // if detected and character then set CPP source
         }

         switch(action)
         {
            case AKEY_EVENT_ACTION_DOWN:
            {
               Kb.push(key, code); Kb.queue(chr, code); // !! queue characters after push !!
            }break;

            case AKEY_EVENT_ACTION_UP:
            {
               Kb.release(key);
            }break;

            case AKEY_EVENT_ACTION_MULTIPLE:
            {
               Kb.push   (key, code); Kb.queue(chr, code); // !! queue characters after push !!
               Kb.release(key      );
            }break;
         }
         // on some systems 'dispatchKeyEvent' will not be called if we return 1
         if(chr              // if we did receive a character then we don't need 'dispatchKeyEvent' anymore
         || key==KB_NAV_BACK // on Asus Transformer Prime pressing this hardware keyboard key will close the app
         )return 1;
      }return 0; // call 'dispatchKeyEvent'

      default:
      {
       /*Int device=AInputEvent_getDeviceId(event);
         LogN(S+"Unknown input type, dev:"+device);*/
      }break;
   }
   return 0;
}
static void SetMAD(Vec2 &mad, Flt min, Flt max) // convert from min..max -> 0..1
{
   mad.x=1.0f/(max-min);
   mad.y=-min*mad.x; // min*mad.x+mad.y=0
}
static void DeviceRemoved(Ptr device_id_ptr) {UInt device_id=UIntPtr(device_id_ptr); REPA(Joypads)if(Joypads[i].id()==device_id){Joypads.remove(i); break;}}
static void DeviceAdded  (Ptr device_id_ptr) {UInt device_id=UIntPtr(device_id_ptr); Bool added; Joypad &joypad=GetJoypad(device_id, added); if(added)
   if(Jni)
      if(JClass InputDevice="android/view/InputDevice")
         if(JMethodID getDevice=Jni.staticFunc(InputDevice, "getDevice", "(I)Landroid/view/InputDevice;"))
            if(JObject device=Jni->CallStaticObjectMethod(InputDevice, getDevice, jint(device_id)))
   {
                        if(JMethodID getName     =Jni.func(InputDevice, "getName"     , "()Ljava/lang/String;"))if(JString name=Jni->CallObjectMethod(device, getName))joypad._name=name.str();
      Int  vendor_id=0; if(JMethodID getVendorId =Jni.func(InputDevice, "getVendorId" , "()I"                 )) vendor_id=Jni->CallIntMethod(device, getVendorId );
      Int product_id=0; if(JMethodID getProductId=Jni.func(InputDevice, "getProductId", "()I"                 ))product_id=Jni->CallIntMethod(device, getProductId);

      if(JMethodID DeviceHasAxis=Jni.staticFunc(ActivityClass, "DeviceHasAxis", "(Landroid/view/InputDevice;I)Z"))
      if(JMethodID DeviceMinAxis=Jni.staticFunc(ActivityClass, "DeviceMinAxis", "(Landroid/view/InputDevice;I)F"))
      if(JMethodID DeviceMaxAxis=Jni.staticFunc(ActivityClass, "DeviceMaxAxis", "(Landroid/view/InputDevice;I)F"))
      {
      #define HAS(axis) (Jni->CallStaticBooleanMethod(ActivityClass, DeviceHasAxis, device(), jint(axis)))
      #define MIN(axis) (Jni->CallStaticFloatMethod  (ActivityClass, DeviceMinAxis, device(), jint(axis)))
      #define MAX(axis) (Jni->CallStaticFloatMethod  (ActivityClass, DeviceMaxAxis, device(), jint(axis)))

         { // stick right
            // Xbox Elite Wireless Controller Series 2, Sony, Nintendo Switch Pro Controller, have X=AMOTION_EVENT_AXIS_Z, Y=AMOTION_EVENT_AXIS_RZ. Samsung EI-GP20 has AMOTION_EVENT_AXIS_RX, AMOTION_EVENT_AXIS_RY
            Byte has[2], has_i=0, check[]={AMOTION_EVENT_AXIS_Z, AMOTION_EVENT_AXIS_RZ, AMOTION_EVENT_AXIS_RX, AMOTION_EVENT_AXIS_RY}; // order important (potential X first, Y next). Prefer AMOTION_EVENT_AXIS_Z/AMOTION_EVENT_AXIS_RZ because they're supported by popular gamepads and also they're mentioned in header for right stick
            FREPA(check)if(HAS(check[i]))
            {
               has[has_i]=check[i]; if(++has_i==2) // found 2 axes
               {
                  joypad._axis_stick_r_x=has[0];
                  joypad._axis_stick_r_y=has[1];
                  break;
               }
            }
         }

         // triggers
         {
            // Xbox Elite Wireless Controller Series 2 uses AMOTION_EVENT_AXIS_BRAKE+AMOTION_EVENT_AXIS_GAS, Sony uses AMOTION_EVENT_AXIS_RX, AMOTION_EVENT_AXIS_RY
            Byte has[2], has_i=0, check[]={AMOTION_EVENT_AXIS_BRAKE, AMOTION_EVENT_AXIS_GAS, AMOTION_EVENT_AXIS_LTRIGGER, AMOTION_EVENT_AXIS_RTRIGGER, AMOTION_EVENT_AXIS_RX, AMOTION_EVENT_AXIS_RY}; // order important (potential X first, Y next).
            FREPA(check)
            {
               Byte axis=check[i]; if(axis!=joypad._axis_stick_r_x && axis!=joypad._axis_stick_r_y && HAS(axis))
               {
                  has[has_i]=axis; if(++has_i==2) // found 2 axes
                  {
                     joypad._axis_trigger_l=has[0];
                     joypad._axis_trigger_r=has[1];
                     break;
                  }
               }
            }
         }

         joypad.setInfo(vendor_id, product_id);
         if(joypad._axis_trigger_l!=0xFF) // has trigger axis
         {
            SetMAD(joypad._axis_trigger_mad[0], MIN(joypad._axis_trigger_l), MAX(joypad._axis_trigger_l)); // this is needed because Sony has triggers in range -1..1
            REPA(joypad._remap)if(joypad._remap[i]==JB_L2)joypad._remap[i]=254; // use 254 as out of range to disable auto-convert
         }
         if(joypad._axis_trigger_r!=0xFF) // has trigger axis
         {
            SetMAD(joypad._axis_trigger_mad[1], MIN(joypad._axis_trigger_r), MAX(joypad._axis_trigger_r)); // this is needed because Sony has triggers in range -1..1
            REPA(joypad._remap)if(joypad._remap[i]==JB_R2)joypad._remap[i]=254; // use 254 as out of range to disable auto-convert
         }

      #undef HAS
      #undef MIN
      #undef MAX
      }
   }
}
/******************************************************************************/
enum ANDROID_STATE
{
   AS_FOCUSED=1<<0,
   AS_STOPPED=1<<1,
   AS_PAUSED =1<<2,
};
static Byte AndroidState=0;
static void SetActive()
{
   Bool    active=((AndroidState&(AS_FOCUSED|AS_STOPPED|AS_PAUSED))==AS_FOCUSED);
   App._maximized=((AndroidState&(           AS_STOPPED|AS_PAUSED))==         0);
   App._minimized=!App._maximized;
   if(App.active()!=active)
   {
      if(active)
      {
         if(Jni && ActivityClass)
         if(JMethodID stopBackgroundService=Jni.func(ActivityClass, "stopBackgroundService", "()V"))
            Jni->CallVoidMethod(Activity, stopBackgroundService);

         LOG2("ResumeSound");
         ResumeSound();
         LOG2("Kb.setVisible");
         Kb.setVisible();
      }
      LOG(S+"App.setActive("+active+")");
      App.setActive(active);
      if(!active)
      {
         if(App.flag&APP_WORK_IN_BACKGROUND)
         {
            if(Jni && ActivityClass)
            if(JMethodID startBackgroundService=Jni.func(ActivityClass, "startBackgroundService", "(Ljava/lang/String;)V"))
            if(JString   j_text=JString(Jni, App.backgroundText()))
               Jni->CallVoidMethod(Activity, startBackgroundService, j_text());
         }else
         {
            LOG2("PauseSound");
            PauseSound();
         }
      }
      LOG2("SetActive finished");
   }
}
static void CmdCallback(android_app *app, int32_t cmd)
{
   switch(cmd)
   {
      default: LOG(S+"CmdCallback ("+cmd+")"); break;

      case APP_CMD_INIT_WINDOW:
      {
         LOG("APP_CMD_INIT_WINDOW");
         if(App._window=app->window)
         {
            if(!Initialized)
            {
               Initialized=true;
               if(!App.create())Exit("Failed to initialize the application"); // something failed, and current state of global variables is in failed mode (they won't be restored at next app launch because Android does not reset the state of global variables) that's why need to do complete reset by killing the process
            }else
            {
               D.androidOpen();
            }
         }
      }break;

      case APP_CMD_TERM_WINDOW:
      {
         LOG("APP_CMD_TERM_WINDOW");
         App._window=null;
         D.androidClose();
      }break;

      case APP_CMD_CONFIG_CHANGED:
      {
         LOG("APP_CMD_CONFIG_CHANGED");
         InputDevices.checkMouseKeyboard();
         Kb.setVisible(); // AFTER 'checkMouseKeyboard'
       /*EGLint w=-1, h=-1;
         eglQuerySurface(display, surface, EGL_WIDTH , &w);
         eglQuerySurface(display, surface, EGL_HEIGHT, &h);
         LOG(S+"EGL: w:"+w+", h:"+h);*/
         UpdateOrientation();
       //UpdateSize       (); // at this stage, old window size may occur, instead we need to check this every frame
      }break;

      case APP_CMD_WINDOW_REDRAW_NEEDED:
      {
         LOG("APP_CMD_WINDOW_REDRAW_NEEDED");
         if(D.created())DrawState();
      }break;

      case APP_CMD_GAINED_FOCUS: LOG("APP_CMD_GAINED_FOCUS"); FlagSet(AndroidState, AS_FOCUSED, true ); SetActive(); break;
      case APP_CMD_LOST_FOCUS  : LOG("APP_CMD_LOST_FOCUS"  ); FlagSet(AndroidState, AS_FOCUSED, false); SetActive(); break;
      case APP_CMD_START       : LOG("APP_CMD_START"       ); FlagSet(AndroidState, AS_STOPPED, false); SetActive(); break;
      case APP_CMD_STOP        : LOG("APP_CMD_STOP"        ); FlagSet(AndroidState, AS_STOPPED, true ); SetActive(); break;
      case APP_CMD_PAUSE       : LOG("APP_CMD_PAUSE"       ); FlagSet(AndroidState, AS_PAUSED , true ); SetActive(); AssetPackManager_onPause (); break;
      case APP_CMD_RESUME      : LOG("APP_CMD_RESUME"      ); FlagSet(AndroidState, AS_PAUSED , false); SetActive(); AssetPackManager_onResume(); break;

      case APP_CMD_DESTROY             : LOG("APP_CMD_DESTROY"             ); break;
      case APP_CMD_SAVE_STATE          : LOG("APP_CMD_SAVE_STATE"          ); if(App.save_state)App.save_state(); break;
      case APP_CMD_WINDOW_RESIZED      : LOG("APP_CMD_WINDOW_RESIZED"      ); break;
      case APP_CMD_CONTENT_RECT_CHANGED: LOG("APP_CMD_CONTENT_RECT_CHANGED"); break;
      case APP_CMD_LOW_MEMORY          : LOG("APP_CMD_LOW_MEMORY"          ); App.lowMemory(); break;
   }
}
/******************************************************************************/
static void JavaInit()
{
   AndroidApp->onAppCmd    =  CmdCallback;
   AndroidApp->onInputEvent=InputCallback;
 //AndroidApp->activity->callbacks->onNativeWindowResized=WindowResized; // this is never called
   JVM         =AndroidApp->activity->vm;
   Activity    =AndroidApp->activity->clazz; // this is not stored as 'JObject' so we can always assign it
   AndroidSDK  =AndroidApp->activity->sdkVersion;
   AssetManager=AndroidApp->activity->assetManager;
   LOG(S);
   LOG(S+"Start - Already Initialized: "+Initialized+", JVM: "+JVM+", JNI:"+Ptr(Jni)+", SDK:"+AndroidSDK);
   Jni.attach();
   if(Jni && Activity)if(ActivityClass=Activity)ActivityClass.makeGlobal();
}
static void JavaShut()
{
   LOG("JavaShut");
   KeyCharacterMapClass.del();
   KeyCharacterMap     .del();
   KeyCharacterMapLoad =null;
   KeyCharacterMapGet  =null;
   DefaultDisplay      .del();
   Activity            =null;
   getRotation         =null;
   vibrate             =null;

#if 0 // we need these even after 'JavaShut'
   ActivityClass        .del();
   ClipboardManagerClass.del();
   ClipboardManager     .del();
   REPAO(EsenthelLocationListener).del();
   LocationManager      .del();
   GPS_PROVIDER.del(); NETWORK_PROVIDER.del();
   getLastKnownLocation=getLatitude=getLongitude=getAltitude=getAccuracy=getSpeed=getTime=requestLocationUpdates=removeUpdates=null;
#endif

   Jni.del();
}
static void JavaGetAppPackage()
{
   LOG("JavaGetAppPackage");
   if(!AndroidPackageName.is() && ActivityClass && Activity)
   if(JMethodID getPackageName=Jni.func(ActivityClass, "getPackageName", "()Ljava/lang/String;"))
   if(JString package_name=Jni->CallObjectMethod(Activity, getPackageName))
      AndroidPackageName=package_name.str();
}
static void JavaGetAppName()
{
   LOG("JavaGetAppName");
   //JClass Context="android/content/Context")
   //JMethodID getApplicationContext=Jni.func(Context, "getApplicationContext", "()Landroid/content/Context;");
   //JMethodID getPackageName=Jni.func(Context, "getPackageName", "()Ljava/lang/String;");
   //JMethodID getApplicationContext=Jni.staticFunc(Context, "getApplicationContext", "()Landroid/content/Context;");
   //JMethodID getPackageName=Jni.staticFunc(Context, "getPackageName", "()Ljava/lang/String;");

   if(Jni && ActivityClass)
   if(JClass FileClass="java/io/File")
   if(JMethodID getAbsolutePath=Jni.func(FileClass, "getAbsolutePath", "()Ljava/lang/String;"))
   {
      // code and assets
      if(JMethodID getPackageCodePath=Jni.func             (ActivityClass, "getPackageCodePath", "()Ljava/lang/String;"))
      if(JString            code_path=Jni->CallObjectMethod(Activity     ,  getPackageCodePath))
         AndroidAppPath=    code_path.str().replace('/', '\\');

      // resources
      //if(JMethodID getPackageResourcePath=Jni.func             (ActivityClass, "getPackageResourcePath", "()Ljava/lang/String;"))
      //if(JString            resource_path=Jni->CallObjectMethod(Activity     ,  getPackageResourcePath)){}

      // app data private
      if(JMethodID getFilesDir     =Jni.func             (ActivityClass, "getFilesDir", "()Ljava/io/File;"))
      if(JObject      files_dir    =Jni->CallObjectMethod(Activity     ,  getFilesDir))
      if(JString      files_dir_str=Jni->CallObjectMethod(files_dir    ,  getAbsolutePath))
         AndroidAppDataPath=files_dir_str.str().replace('/', '\\');

      // app data public
      if(JMethodID getExternalFilesDir      =Jni.func             (ActivityClass     , "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;"))
      if(JObject      external_files_dir    =Jni->CallObjectMethod(Activity          ,  getExternalFilesDir, null))
      if(JString      external_files_dir_str=Jni->CallObjectMethod(external_files_dir,  getAbsolutePath))
         AndroidAppDataPublicPath=external_files_dir_str.str().replace('/', '\\');

      // app cache
      if(JMethodID getCacheDir     =Jni.func             (ActivityClass, "getCacheDir", "()Ljava/io/File;"))
      if(JObject      files_dir    =Jni->CallObjectMethod(Activity     ,  getCacheDir))
      if(JString      files_dir_str=Jni->CallObjectMethod(files_dir    ,  getAbsolutePath))
         AndroidAppCachePath=files_dir_str.str().replace('/', '\\');

      // public
      if(JClass    Environment="android/os/Environment")
      if(JMethodID getExternalStorageDirectory    =Jni. staticFunc            (Environment, "getExternalStorageDirectory", "()Ljava/io/File;"))
      if(JObject      externalStorageDirectory    =Jni->CallStaticObjectMethod(Environment,  getExternalStorageDirectory))
      if(JString      externalStorageDirectory_str=Jni->CallObjectMethod      (externalStorageDirectory, getAbsolutePath))
         AndroidPublicPath=externalStorageDirectory_str.str().replace('/', '\\');

      // SD Card
      // this method gets array of files using 'getExternalFilesDirs', and ignores 'AndroidAppDataPublicPath' obtained with 'getExternalFilesDir', then it removes the shared end like "external/Android/App", "sd_card/Android/App" to leave only "sd_card"
      if(JMethodID getExternalFilesDirs=Jni.func             (ActivityClass, "getExternalFilesDirs", "(Ljava/lang/String;)[Ljava/io/File;"))
      if(JObjectArray              dirs=Jni->CallObjectMethod(Activity     ,  getExternalFilesDirs, null))
      {
         Int  length=dirs.elms();
         FREP(length)
            if(JObject dir=dirs[i]) // check this because it can be null
            if(JString dir_path=Jni->CallObjectMethod(dir, getAbsolutePath))
         {
            Str path=dir_path.str(); if(path.is() && !EqualPath(path, AndroidAppDataPublicPath))
            {
               path.replace('/', '\\');
               Int same=0;
               FREPA(path)
               {
                  Char a=path[path.length()-1-i], b=AndroidAppDataPublicPath[AndroidAppDataPublicPath.length()-1-i]; // last chars
                  if(IsSlash(a) && IsSlash(b))same=i+1;else if(a!=b)break;
               }
               path.removeLast(same);
               AndroidSDCardPath=path;
               break;
            }
         }
      }

      // after having SD card path, call Java to setup allowed sharing paths
      if(JMethodID setSharingPaths=Jni.func(ActivityClass, "setSharingPaths", "(Ljava/lang/String;)V"))
      if(JString   sd_card=UnixPath(AndroidSDCardPath))
         Jni->CallVoidMethod(Activity, setSharingPaths, sd_card());
   }
   if(LogInit && AndroidPublicPath.is() && !EqualPath(AndroidPublicPath, GetPath(LogName()))){Str temp=LogName(); LogN(S+"Found public storage directory and continuing log there: "+AndroidPublicPath); LogName(Str(AndroidPublicPath).tailSlash(true)+ENGINE_NAME " Log.txt"); LogN(S+"Continuing log from: "+temp);}
}
static void JavaLooper()
{
   // prepare the Looper, this is needed for 'ClipboardManager' and 'LocationListener'
   LOG("JavaLooper");
   if(Jni)
   if(JClass LooperClass="android/os/Looper")
   if(JMethodID prepare=Jni.staticFunc(LooperClass, "prepare", "()V"))
   {
      Jni->CallStaticVoidMethod(LooperClass, prepare);
      if(Jni->ExceptionCheck())
      {
      #if DEBUG
         Jni->ExceptionDescribe();
      #endif
         Jni->ExceptionClear();
      }
   }
}
static void JavaGetInput()
{
   LOG("JavaGetInput");

   if(!vibrate)
      if(Jni && ActivityClass)
         vibrate=Jni.staticFunc(ActivityClass, "vibrate", "(II)V");
}
static void JavaDisplay()
{
   LOG("JavaDisplay");
   if(!DefaultDisplay)
      if(Jni && ActivityClass)
   {
      // Get WindowManager
      if(JClass WindowManagerClass="android/view/WindowManager")
      if(JMethodID getWindowManager=Jni.func(ActivityClass, "getWindowManager", "()Landroid/view/WindowManager;"))
      if(JObject window_manager=Jni->CallObjectMethod(Activity, getWindowManager))
      {
         // Get DefaultDisplay
         if(JClass Display="android/view/Display")
         if(JMethodID getDefaultDisplay=Jni.func(WindowManagerClass, "getDefaultDisplay", "()Landroid/view/Display;"))
         if(DefaultDisplay=Jni->CallObjectMethod(window_manager, getDefaultDisplay))if(DefaultDisplay.makeGlobal())
         {
            // Get Display methods
            getRotation=Jni.func(Display, "getRotation", "()I"); // set this as the last one so only one 'if' can be performed in 'UpdateOrientation'
         }
      }
   }
   UpdateOrientation();
}
static void JavaKeyboard()
{
   LOG("JavaKeyboard");
   if(!KeyCharacterMapGet)
      if(Jni)
      if(KeyCharacterMapClass="android/view/KeyCharacterMap")
      if(KeyCharacterMapLoad=Jni.staticFunc(KeyCharacterMapClass, "load", "(I)Landroid/view/KeyCharacterMap;"))
         KeyCharacterMapGet=Jni.func(KeyCharacterMapClass, "get", "(II)I");
}
static void JavaClipboard()
{
   LOG("JavaClipboard");
   if(!ClipboardManager)
      if(Jni && ActivityClass)
      if(JClass ContextClass="android/content/Context")
      if(JFieldID CLIPBOARD_SERVICEField=Jni->GetStaticFieldID(ContextClass, "CLIPBOARD_SERVICE", "Ljava/lang/String;"))
      if(JObject CLIPBOARD_SERVICE=Jni->GetStaticObjectField(ContextClass, CLIPBOARD_SERVICEField))
      if(ClipboardManagerClass="android/text/ClipboardManager")if(ClipboardManagerClass.makeGlobal()) // android/content/ClipboardManager
      if(JMethodID getSystemService=Jni.func(ActivityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;"))
      {
         ClipboardManager=Jni->CallObjectMethod(Activity, getSystemService, CLIPBOARD_SERVICE());
         if(Jni->ExceptionCheck())
         {
         #if DEBUG
            Jni->ExceptionDescribe();
         #endif
            ClipboardManager.clear(); ClipboardManagerClass.clear(); Jni->ExceptionClear(); LOG("ClipboardManager failed");
         }else ClipboardManager.makeGlobal();
      }
}
static void JavaLocation()
{
   LOG("JavaLocation");
   if(!LocationManager && Jni && ActivityClass)
   if(JMethodID getClassLoader     =Jni.func(ActivityClass, "getClassLoader", "()Ljava/lang/ClassLoader;"))
   if(JObject      ClassLoader     =Jni->CallObjectMethod(Activity, getClassLoader))
   if(JClass       ClassLoaderClass="java/lang/ClassLoader")
   if(JMethodID loadClass=Jni.func(ClassLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;"))
   if(JClass ContextClass="android/content/Context")
   if(JFieldID LOCATION_SERVICEField=Jni->GetStaticFieldID(ContextClass, "LOCATION_SERVICE", "Ljava/lang/String;"))
   if(JObject LOCATION_SERVICE=Jni->GetStaticObjectField(ContextClass, LOCATION_SERVICEField))
   if(JClass LocationManagerClass="android/location/LocationManager")
   if(JMethodID getSystemService=Jni.func(ActivityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;"))
   {
      LocationManager=Jni->CallObjectMethod(Activity, getSystemService, LOCATION_SERVICE());
      if(Jni->ExceptionCheck())
      {
      #if DEBUG
         Jni->ExceptionDescribe();
      #endif
         LocationManager.clear(); LocationManagerClass.clear(); Jni->ExceptionClear(); LOG("LocationManager failed");
      }else LocationManager.makeGlobal();

      if(LocationManager)
      if(JClass LocationClass="android/location/Location")
      if(JFieldID     GPS_PROVIDERField=Jni->GetStaticFieldID(LocationManagerClass,     "GPS_PROVIDER", "Ljava/lang/String;"))
      if(JFieldID NETWORK_PROVIDERField=Jni->GetStaticFieldID(LocationManagerClass, "NETWORK_PROVIDER", "Ljava/lang/String;"))
      if(    GPS_PROVIDER      =Jni->GetStaticObjectField(LocationManagerClass,     GPS_PROVIDERField))if(    GPS_PROVIDER.makeGlobal())
      if(NETWORK_PROVIDER      =Jni->GetStaticObjectField(LocationManagerClass, NETWORK_PROVIDERField))if(NETWORK_PROVIDER.makeGlobal())
      if(getLastKnownLocation  =Jni.func(LocationManagerClass, "getLastKnownLocation"  , "(Ljava/lang/String;)Landroid/location/Location;"))
      if(getLatitude           =Jni.func(LocationClass       , "getLatitude"           , "()D"))
      if(getLongitude          =Jni.func(LocationClass       , "getLongitude"          , "()D"))
      if(getAltitude           =Jni.func(LocationClass       , "getAltitude"           , "()D"))
      if(getAccuracy           =Jni.func(LocationClass       , "getAccuracy"           , "()F"))
      if(getSpeed              =Jni.func(LocationClass       , "getSpeed"              , "()F"))
      if(getTime               =Jni.func(LocationClass       , "getTime"               , "()J")) // set this as the last one so only one 'if' can be performed in 'UpdateLocation'
      if(requestLocationUpdates=Jni.func(LocationManagerClass, "requestLocationUpdates", "(Ljava/lang/String;JFLandroid/location/LocationListener;)V"))
      if(removeUpdates         =Jni.func(LocationManagerClass, "removeUpdates"         , "(Landroid/location/LocationListener;)V"))
      if(JString EsenthelLocationListenerName=AndroidPackageName+"/EsenthelActivity$EsenthelLocationListener")
      {
         JClass EsenthelLocationListenerClass=(jclass)Jni->CallObjectMethod(ClassLoader, loadClass, EsenthelLocationListenerName());
         if(Jni->ExceptionCheck()){EsenthelLocationListenerClass.clear(); Jni->ExceptionClear(); LOG("EsenthelLocationListenerClass failed");}

         if(EsenthelLocationListenerClass)
         if(JMethodID EsenthelLocationListenerCtor=Jni.func(EsenthelLocationListenerClass, "<init>", "(Z)V"))
         REP(2)
         {
            EsenthelLocationListener[i]=Jni->NewObject(EsenthelLocationListenerClass, EsenthelLocationListenerCtor, jboolean(i)); // set this as the last one so only one 'if' can be performed in 'SetLocationRefresh'
            if(Jni->ExceptionCheck()){EsenthelLocationListener[i].clear(); Jni->ExceptionClear(); LOG(S+"EsenthelLocationListener["+i+"] failed");}else EsenthelLocationListener[i].makeGlobal();
         }
      }
   }

   if(Jni)Jni->ExceptionClear(); // 'ExceptionClear' must be called in case some func returned null
   UpdateLocation(Jni);
}
/******************************************************************************/
static void InitSensor()
{
   LOG("InitSensor");
   if(!SensorEventQueue)
      if(ASensorManager *sensor_manager=ASensorManager_getInstance())
         SensorEventQueue=ASensorManager_createEventQueue(sensor_manager, AndroidApp->looper, LOOPER_ID_USER, null, null);
}
static void ShutSensor()
{
   LOG("ShutSensor");
   InputDevices.acquire(false); // disable all sensors just in case
   if(SensorEventQueue)
   {
      if(ASensorManager *sensor_manager=ASensorManager_getInstance())
         ASensorManager_destroyEventQueue(sensor_manager, SensorEventQueue);
      SensorEventQueue=null;
   }
}
/******************************************************************************/
static void InitKeyMap()
{
   KeyMap[AKEYCODE_0]=KB_0;
   KeyMap[AKEYCODE_1]=KB_1;
   KeyMap[AKEYCODE_2]=KB_2;
   KeyMap[AKEYCODE_3]=KB_3;
   KeyMap[AKEYCODE_4]=KB_4;
   KeyMap[AKEYCODE_5]=KB_5;
   KeyMap[AKEYCODE_6]=KB_6;
   KeyMap[AKEYCODE_7]=KB_7;
   KeyMap[AKEYCODE_8]=KB_8;
   KeyMap[AKEYCODE_9]=KB_9;

   KeyMap[AKEYCODE_A]=KB_A;
   KeyMap[AKEYCODE_B]=KB_B;
   KeyMap[AKEYCODE_C]=KB_C;
   KeyMap[AKEYCODE_D]=KB_D;
   KeyMap[AKEYCODE_E]=KB_E;
   KeyMap[AKEYCODE_F]=KB_F;
   KeyMap[AKEYCODE_G]=KB_G;
   KeyMap[AKEYCODE_H]=KB_H;
   KeyMap[AKEYCODE_I]=KB_I;
   KeyMap[AKEYCODE_J]=KB_J;
   KeyMap[AKEYCODE_K]=KB_K;
   KeyMap[AKEYCODE_L]=KB_L;
   KeyMap[AKEYCODE_M]=KB_M;
   KeyMap[AKEYCODE_N]=KB_N;
   KeyMap[AKEYCODE_O]=KB_O;
   KeyMap[AKEYCODE_P]=KB_P;
   KeyMap[AKEYCODE_Q]=KB_Q;
   KeyMap[AKEYCODE_R]=KB_R;
   KeyMap[AKEYCODE_S]=KB_S;
   KeyMap[AKEYCODE_T]=KB_T;
   KeyMap[AKEYCODE_U]=KB_U;
   KeyMap[AKEYCODE_V]=KB_V;
   KeyMap[AKEYCODE_W]=KB_W;
   KeyMap[AKEYCODE_X]=KB_X;
   KeyMap[AKEYCODE_Y]=KB_Y;
   KeyMap[AKEYCODE_Z]=KB_Z;

   KeyMap[131]=KB_F1;
   KeyMap[132]=KB_F2;
   KeyMap[133]=KB_F3;
   KeyMap[134]=KB_F4;
   KeyMap[135]=KB_F5;
   KeyMap[136]=KB_F6;
   KeyMap[137]=KB_F7;
   KeyMap[138]=KB_F8;
   KeyMap[139]=KB_F9;
   KeyMap[140]=KB_F10;
   KeyMap[141]=KB_F11;
   KeyMap[142]=KB_F12;

   KeyMap[111           ]=KB_ESC;
   KeyMap[AKEYCODE_ENTER]=KB_ENTER;
   KeyMap[AKEYCODE_SPACE]=KB_SPACE;
   KeyMap[AKEYCODE_DEL  ]=KB_BACK;
   KeyMap[AKEYCODE_TAB  ]=KB_TAB;

   KeyMap[113                 ]=KB_LCTRL;
   KeyMap[114                 ]=KB_RCTRL;
   KeyMap[AKEYCODE_SHIFT_LEFT ]=KB_LSHIFT;
   KeyMap[AKEYCODE_SHIFT_RIGHT]=KB_RSHIFT;
   KeyMap[AKEYCODE_ALT_LEFT   ]=KB_LALT;
   KeyMap[AKEYCODE_ALT_RIGHT  ]=KB_RALT;
   KeyMap[117                 ]=KB_LWIN;
   KeyMap[118                 ]=KB_RWIN;
   KeyMap[AKEYCODE_MENU       ]=KB_MENU;

   KeyMap[AKEYCODE_DPAD_UP   ]=KB_UP;
   KeyMap[AKEYCODE_DPAD_DOWN ]=KB_DOWN;
   KeyMap[AKEYCODE_DPAD_LEFT ]=KB_LEFT;
   KeyMap[AKEYCODE_DPAD_RIGHT]=KB_RIGHT;

   KeyMap[124               ]=KB_INS;
   KeyMap[112               ]=KB_DEL;
   KeyMap[122               ]=KB_HOME;
   KeyMap[123               ]=KB_END;
   KeyMap[AKEYCODE_PAGE_UP  ]=KB_PGUP;
   KeyMap[AKEYCODE_PAGE_DOWN]=KB_PGDN;

   KeyMap[AKEYCODE_MINUS        ]=KB_SUB;
   KeyMap[AKEYCODE_EQUALS       ]=KB_EQUAL;
   KeyMap[AKEYCODE_LEFT_BRACKET ]=KB_LBRACKET;
   KeyMap[AKEYCODE_RIGHT_BRACKET]=KB_RBRACKET;
   KeyMap[AKEYCODE_SEMICOLON    ]=KB_SEMICOLON;
   KeyMap[AKEYCODE_APOSTROPHE   ]=KB_APOSTROPHE;
   KeyMap[AKEYCODE_COMMA        ]=KB_COMMA;
   KeyMap[AKEYCODE_PERIOD       ]=KB_DOT;
   KeyMap[AKEYCODE_SLASH        ]=KB_SLASH;
   KeyMap[AKEYCODE_BACKSLASH    ]=KB_BACKSLASH;
   KeyMap[AKEYCODE_GRAVE        ]=KB_TILDE;
   KeyMap[120                   ]=KB_PRINT;
   KeyMap[121                   ]=KB_PAUSE;
   KeyMap[115                   ]=KB_CAPS;
   KeyMap[143                   ]=KB_NUM;
   KeyMap[116                   ]=KB_SCROLL;

   KeyMap[154]=KB_NPDIV;
   KeyMap[155]=KB_NPMUL;
   KeyMap[156]=KB_NPSUB;
   KeyMap[157]=KB_NPADD;
   KeyMap[158]=KB_NPDEL;

   KeyMap[144]=KB_NP0;
   KeyMap[145]=KB_NP1;
   KeyMap[146]=KB_NP2;
   KeyMap[147]=KB_NP3;
   KeyMap[148]=KB_NP4;
   KeyMap[149]=KB_NP5;
   KeyMap[150]=KB_NP6;
   KeyMap[151]=KB_NP7;
   KeyMap[152]=KB_NP8;
   KeyMap[153]=KB_NP9;

   KeyMap[AKEYCODE_VOLUME_DOWN     ]=KB_VOL_DOWN;
   KeyMap[AKEYCODE_VOLUME_UP       ]=KB_VOL_UP;
   KeyMap[AKEYCODE_MUTE            ]=KB_VOL_MUTE;
   KeyMap[AKEYCODE_BACK            ]=KB_NAV_BACK;
   KeyMap[AKEYCODE_MEDIA_PREVIOUS  ]=KB_MEDIA_PREV;
   KeyMap[AKEYCODE_MEDIA_NEXT      ]=KB_MEDIA_NEXT;
   KeyMap[AKEYCODE_MEDIA_PLAY_PAUSE]=KB_MEDIA_PLAY;
   KeyMap[AKEYCODE_MEDIA_STOP      ]=KB_MEDIA_STOP;
   KeyMap[AKEYCODE_SEARCH          ]=KB_FIND;

   KeyMap[168]=KB_ZOOM_IN ; // AKEYCODE_ZOOM_IN
   KeyMap[169]=KB_ZOOM_OUT; // AKEYCODE_ZOOM_OUT

   SetMem(JoyMap, 255);
   JoyMap[AKEYCODE_BUTTON_A     ]=JB_A;
   JoyMap[AKEYCODE_BUTTON_B     ]=JB_B;
   JoyMap[AKEYCODE_BUTTON_X     ]=JB_X;
   JoyMap[AKEYCODE_BUTTON_Y     ]=JB_Y;
   JoyMap[AKEYCODE_BUTTON_L1    ]=JB_L1;
   JoyMap[AKEYCODE_BUTTON_R1    ]=JB_R1;
   JoyMap[AKEYCODE_BUTTON_L2    ]=JB_L2;
   JoyMap[AKEYCODE_BUTTON_R2    ]=JB_R2;
   JoyMap[AKEYCODE_BUTTON_THUMBL]=JB_L3;
   JoyMap[AKEYCODE_BUTTON_THUMBR]=JB_R3;
   JoyMap[AKEYCODE_BUTTON_SELECT]=JB_SELECT;
   JoyMap[AKEYCODE_BUTTON_START ]=JB_START;
}
/******************************************************************************/
static Bool Loop()
{
   LOG2("ALooper_pollAll");

   Bool wait_end_set=false; Int wait=(App.active() ? App.active_wait : 0); UInt wait_end; // don't wait for !active, because we already wait after the event loop, and that would make 2 waits

   Int id, events; android_poll_source *source;
wait:
   while((id=ALooper_pollAll(wait, null, &events, (void**)&source))>=0) // process all events, using negative 'wait' for 'ALooper_pollAll' means unlimited wait
   {
      LOG2(S+"ALooper Source:"+Ptr(source)+", id:"+id);
      if(source)source->process(AndroidApp, source); // process this event
      LOG2(S+"ALooper processed");

      if(id==LOOPER_ID_USER) // sensor data
      {
         ASensorEvent event;
      #if DEBUG
         if(!SensorEventQueue)LOG("Received sensor event but the 'SensorEventQueue' is null");
      #endif
         while(ASensorEventQueue_getEvents(SensorEventQueue, &event, 1)>0)switch(event.type)
         {
            case ASENSOR_TYPE_ACCELEROMETER : AccelerometerValue.set(-event.acceleration.x, -event.acceleration.y,  event.acceleration.z); break;
            case ASENSOR_TYPE_GYROSCOPE     :     GyroscopeValue.set( event.vector      .x,  event.vector      .y, -event.vector      .z); break;
            case ASENSOR_TYPE_MAGNETIC_FIELD:  MagnetometerValue.set( event.magnetic    .x,  event.magnetic    .y, -event.magnetic    .z); break;
         }
      }

      wait=0; // don't wait for next events, in case app 'activate' status changed or it was requested to be closed or destroyed
      // no need to check for 'App._close' or 'AndroidApp->destroyRequested' here, because we will do it below since we're setting wait=0 we don't risk with unlimited waits
   }
   if(App._close) // first check if we want to close manually
   {
      App.del(); // manually call shut down
      ExitNow(); // do complete reset (including state of global variables) by killing the process
   }
   if(AndroidApp->destroyRequested)return false; // this is triggered by 'ANativeActivity_finish', just break out of the loop so app can get minimized
   if(!App.active()) // we may need to wait
      if(wait=((App.flag&APP_WORK_IN_BACKGROUND) ? App.background_wait : -1)) // how long
   {
      if(wait>0) // finite wait
      {
         if(wait_end_set) // if we already set the end time limit
         {
            wait=wait_end-Time.curTimeMs(); if(wait<=0)goto stop; // calculate remaining time
         }else
         {
            wait_end=Time.curTimeMs()+wait;
            wait_end_set=true;
         }
      }//else wait=-1; // use negative wait to force unlimited wait in 'ALooper_pollAll', no need for this, because 'wait' is already negative
      goto wait;
   }
stop:

   // process input
   Ms._delta_rel.x= Ms._delta_pixeli_clp.x;
   Ms._delta_rel.y=-Ms._delta_pixeli_clp.y;

   LOG2(S+"AndroidApp->window:"+(AndroidApp->window!=null));
#if DEBUG
   if(!eglGetCurrentContext())LOG("No current EGL Context available on the main thread");
#endif
   UpdateSize();
   App.update();
   return true;
}
/******************************************************************************/
// PLAY ASSET DELIVERY
/******************************************************************************
commands for testing PAD locally without uploading to Play Store - https://developer.android.com/guide/playcore/asset-delivery/test
bundletool-all downloaded from https://github.com/google/bundletool/releases
For testing "Esenthel/Project/Project.vcxproj" first have to turn on all #PlayAssetDelivery in "build.gradle" and "settings.gradle"
Rebuild Project
May need to uninstall app on device if asset pack data was changed
Run powershell in "Project/Android" path
Copy all commands below and paste (Ctrl+V) to powershell, press Enter:

./gradlew bundleRelease
del app/build/output.apks
java -jar C:/bundletool-all.jar build-apks --bundle=app/build/outputs/bundle/release/app-release.aab --output=app/build/output.apks --local-testing
java -jar C:/bundletool-all.jar install-apks --apks=app/build/output.apks --adb=C:/Progs/AndroidSDK/platform-tools/adb.exe
/******************************************************************************/
struct PlayAssetDelivery
{
   Bool create(Int asset_packs, Cipher *project_cipher);
   void del   ();

   Bool update();

   Bool  updating ()C {return _updating ;}
   ULong completed()C {return _completed;}
   ULong total    ()C {return _total    ;}

#if !EE_PRIVATE
private:
#endif
   Bool   _updating;
   Int     asset_packs;
   ULong  _completed, _total;
   Cipher *cipher;

#if EE_PRIVATE
   void zero() {_updating=false; asset_packs=0; _completed=_total=0; cipher=null;}
#endif

  ~PlayAssetDelivery() {del();}
   PlayAssetDelivery();
};
/******************************************************************************/
static void Error(AssetPackErrorCode error)
{
   CChar8 *msg; 
   switch(error)
   {
      case ASSET_PACK_APP_UNAVAILABLE      : msg="App Info unavailable"; break;
      case ASSET_PACK_UNAVAILABLE          : msg="Asset pack unavailable"; break;
      case ASSET_PACK_INVALID_REQUEST      : msg="Invalid request"; break;
      case ASSET_PACK_DOWNLOAD_NOT_FOUND   : msg="Download not found"; break;
      case ASSET_PACK_API_NOT_AVAILABLE    : msg="API unavailable"; break;
      case ASSET_PACK_NETWORK_ERROR        : msg="Network error"; break;
      case ASSET_PACK_ACCESS_DENIED        : msg="Access denied"; break;
      case ASSET_PACK_INSUFFICIENT_STORAGE : msg="Insufficient storage"; break;
      case ASSET_PACK_PLAY_STORE_NOT_FOUND : msg="Google Play Store app not found"; break;
      case ASSET_PACK_NETWORK_UNRESTRICTED : msg="NETWORK_UNRESTRICTED"; break;
      case ASSET_PACK_APP_NOT_OWNED        : msg="App isn't owned"; break;
      case ASSET_PACK_INTERNAL_ERROR       : msg="Internal Error"; break;
      case ASSET_PACK_INITIALIZATION_NEEDED: msg="Initialization needed"; break;
      case ASSET_PACK_INITIALIZATION_FAILED: msg="Initialization failed"; break;
      default                              : msg="Unknown error"; break;
   }
   Exit(S+"Google Play Asset Delivery failed:\n"+msg);
}
struct AssetPack
{
   AssetPackDownloadState *state=null;
  ~AssetPack() {AssetPackDownloadState_destroy(state);}
};
static CChar8* AssetPackName(Char8 (&name)[16], Int i)
{
   Char8 temp[256]; Set(name, "Data"); Append(name, TextInt(i, temp));
   return name;
}
PlayAssetDelivery::PlayAssetDelivery() {zero();}
Bool PlayAssetDelivery::create(Int asset_packs, Cipher *project_cipher)
{
   del();

   const int MAX_ASSET_PACKS=16;
   if(asset_packs<=0)return true;
   DYNAMIC_ASSERT(asset_packs<=MAX_ASSET_PACKS, "asset_packs>MAX_ASSET_PACKS");
   T.asset_packs=asset_packs;
   T.cipher=project_cipher;

   auto error=AssetPackManager_init(AndroidApp->activity->vm, AndroidApp->activity->clazz); if(error!=ASSET_PACK_NO_ERROR)Error(error);
    Char8  names   [MAX_ASSET_PACKS][16];
   CChar8 *name_ptr[MAX_ASSET_PACKS];
   FREP(asset_packs)name_ptr[i]=AssetPackName(names[i], i);
   error=AssetPackManager_requestInfo(name_ptr, asset_packs); if(error!=ASSET_PACK_NO_ERROR)Error(error);

   return update();
}
void PlayAssetDelivery::del()
{
   AssetPackManager_destroy();
   zero();
}
Bool PlayAssetDelivery::update()
{
  _completed=_total=0;
   Int finished=0, updating=0;
   FREP(asset_packs)
   {
      Char8 name[16]; AssetPackName(name, i);
      AssetPack asset_pack; auto error=AssetPackManager_getDownloadState(name, &asset_pack.state); if(error!=ASSET_PACK_NO_ERROR || !asset_pack.state)Error(error);
      AssetPackDownloadStatus status=AssetPackDownloadState_getStatus(asset_pack.state);
   #if DEBUG && 0
      LogN(S+"state"+i+"   "+status);
   #endif
      switch(status)
      {
         case ASSET_PACK_DOWNLOAD_CANCELED:
         case ASSET_PACK_NOT_INSTALLED:
            {CChar8 *name_ptr[]={name}; error=AssetPackManager_requestDownload(name_ptr, 1); if(error!=ASSET_PACK_NO_ERROR)Error(error);} break;
         case ASSET_PACK_WAITING_FOR_WIFI  : error=AssetPackManager_showCellularDataConfirmation(AndroidApp->activity->clazz); if(error!=ASSET_PACK_NO_ERROR)Error(error); break;
         case ASSET_PACK_DOWNLOAD_COMPLETED: finished++; break;
         case ASSET_PACK_TRANSFERRING      : updating++; break;
      }
     _completed+=AssetPackDownloadState_getBytesDownloaded     (asset_pack.state);
     _total    +=AssetPackDownloadState_getTotalBytesToDownload(asset_pack.state);
   }
   if(finished==asset_packs) // all finished
   {
      FREP(asset_packs) // process in order
      {
         Char8 name[16]; AssetPackName(name, i);
         AssetPackLocation *location=null; auto error=AssetPackManager_getAssetPackLocation(name, &location); if(error!=ASSET_PACK_NO_ERROR || !location)Error(error);
      #if DEBUG && 0
         AssetPackStorageMethod storage_method=AssetPackLocation_getStorageMethod(location); LogN(S+"storage:"+storage_method);
      #endif
         auto path=AssetPackLocation_getAssetsPath(location); if(!Is(path))Exit("Empty Asset Pack Path");
         Paks.add(Str(path).tailSlash(true)+name+".pak", cipher, false);
         AssetPackLocation_destroy(location); // !! after this can't use 'path' anymore !!
      }
      Paks.rebuild();
      del(); return true;
   }
   if(finished+updating==asset_packs)_updating=true; // all finished downloading, but some are still updating
   return false;
}
/******************************************************************************/
static Bool  LoadAndroidAssetPacksUpdate() {return true;}
static void  LoadAndroidAssetPacksDraw  () {if(D.created())D.clearCol();}
static State LoadAndroidAssetPacksState (LoadAndroidAssetPacksUpdate, LoadAndroidAssetPacksDraw);
static Str   MB(ULong i) {return TextInt(i>>20, -1, 3);}
void LoadAndroidAssetPacks(Int asset_packs, Cipher *cipher)
{
   PlayAssetDelivery pad; if(!pad.create(asset_packs, cipher))
   {
      auto   app_flag=App.flag       ; FlagDisable(App.flag, APP_WORK_IN_BACKGROUND);
      Int active_wait=App.active_wait;             App.active_wait=10;
      State   *active=StateActive, *next=StateNext; StateActive=StateNext=&LoadAndroidAssetPacksState;
      Flt time=Time.curTime()+1; // show message only after some time to avoid blinking if startup is fast
   again:
      if(Time.curTime()>=time)
      {
         if(pad.updating())Overlay("Updating Asset Packs\n\n");else
         {
            Str s="Downloading Asset Packs\n";
            if(pad.total())s+=S+MB(pad.completed())+" / "+MB(pad.total())+" MB"; s+='\n';
            if(pad.total())s+=S+pad.completed()*100/pad.total()+'%';
            Overlay(s);
         }
      }
      if(!Loop      ())Exit(); // destroy requested
      if(!pad.update())goto again;
      Overlay(S); // hide overlay
      App.flag       =   app_flag;
      App.active_wait=active_wait;
      StateActive=active; StateNext=next;
   }
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
extern "C"
{

JNIEXPORT void JNICALL Java_com_esenthel_Native_connected(JNIEnv *env, jclass clazz, jboolean supports_items, jboolean supports_subs);
JNIEXPORT void JNICALL Java_com_esenthel_Native_location (JNIEnv *env, jclass clazz, jboolean gps, jobject location) {JNI jni(env); UpdateLocation(location, gps!=0, jni);}
JNIEXPORT void JNICALL Java_com_esenthel_Native_resized  (JNIEnv *env, jclass clazz, jint w, jint h, jint visible_x, jint visible_y, jint visible_w, jint visible_h)
{
   Int l_size=visible_x,
       t_size=visible_y,
       r_size=w-(visible_x+visible_w),
       b_size=h-(visible_y+visible_h), max_size=Max(l_size, r_size, t_size, b_size);
   if(b_size>=max_size)Kb._recti.set(       0, h-b_size,      w,      h);else // bottom size is the biggest
   if(t_size>=max_size)Kb._recti.set(       0,        0,      w, t_size);else // top    size is the biggest
   if(l_size>=max_size)Kb._recti.set(       0,        0, l_size,      h);else // left   size is the biggest
                       Kb._recti.set(w-r_size,        0,      w,      h);     // right  size is the biggest
}

JNIEXPORT void JNICALL Java_com_esenthel_Native_deviceAdded  (JNIEnv *env, jclass clazz, jint device_id) {App._callbacks.add(DeviceAdded  , Ptr(device_id));} // may be called on a secondary thread
JNIEXPORT void JNICALL Java_com_esenthel_Native_deviceRemoved(JNIEnv *env, jclass clazz, jint device_id) {App._callbacks.add(DeviceRemoved, Ptr(device_id));} // may be called on a secondary thread

}
/******************************************************************************/
//jint JNI_OnLoad(JavaVM *vm, void *reserved) {LOG("JNI_OnLoad"); return JNI_VERSION_1_6;} don't use since it's called not before 'android_main' but during the first 'ALooper_pollAll'
void android_main(android_app *app)
{
   // !! call before 'JavaInit' !!
   if(BackgroundThread.created()) // stop thread if running
   {
      BackgroundThread.stop(); // request thread to be stopped
      BackgroundThread._resume.on(); // wake up, Warning: here we reuse its 'SyncEvent' because it's used only for pausing/resuming
      BackgroundThread.del(); // wait and delete
   }

   // strip prevention
   Java_com_esenthel_Native_connected(null, null, Store.supportsItems(), Store.supportsSubscriptions()); // don't remove this, or the linker will strip all Java Native functions

   // init
   if(LogInit)
   {
      if(FExistSystem("/sdcard"        ))LogName(        "/sdcard/" ENGINE_NAME " Log.txt");else
      if(FExistSystem("/mnt/sdcard"    ))LogName(    "/mnt/sdcard/" ENGINE_NAME " Log.txt");else
      if(FExistSystem("/mnt/sdcard-ext"))LogName("/mnt/sdcard-ext/" ENGINE_NAME " Log.txt");
   }
   AndroidApp=app;
   JavaInit();
   if(!Initialized) // these can be called only once
   {
      InitKeyMap       ();
      JavaGetAppPackage();
      JavaGetAppName   ();
   }
   JavaLooper   ();
   JavaGetInput ();
   JavaDisplay  ();
   JavaKeyboard ();
   JavaClipboard();
   JavaLocation ();
   InitSensor   ();
   LOG("LoopStart");
   for(; Loop(); );
   LOG("LoopEnd");
   KeyboardLoaded=false;
   ShutSensor(); // !! call before 'JavaShut' !!
   JavaShut(); // !! call before 'BackgroundThread' !!

   // app had input callback set, if there were some touches then they won't be unregistered now since app will be deleted, and because memory state can be preserved on Android, then manually remove them, without this touches may be preserved when rotating the screen, even though we're no longer touching it
   REPA(Touches)
   {
      Touch &touch=Touches[i];
      if(touch.on()) // if was pressed then release it (so application can process release)
      {
         touch._remove= true;
         touch._state|= BS_RELEASED;
         touch._state&=~BS_ON;
      }else // if not pressed then just remove it
      {
         Touches.remove(i, true);
      }
   }

   if((App.flag&APP_WORK_IN_BACKGROUND) && App.background_wait>=0)
   {
      LOG("BackgroundThread.create");
      BackgroundThread.create(BackgroundFunc, null, 0, false, "BackgroundThread"); // !! call after 'JavaShut' !!
   }
   LOG("End");
}
/******************************************************************************/
#endif // ANDROID
/******************************************************************************/
