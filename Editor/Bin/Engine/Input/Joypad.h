/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'Joypads' container to access Joypads input.

/******************************************************************************/
#if EE_PRIVATE
enum JP_NAME_TYPE : Byte
{
   JP_NAME         ,
   JP_NAME_NINTENDO,
   JP_NAME_SONY    ,
   JP_NAMES        ,
};
#endif
enum JOYPAD_BUTTON : Byte // button indexes as defined for XInput/Xbox/NintendoSwitch controllers
{
   JB_A    , // A
   JB_B    , // B
   JB_X    , // X
   JB_Y    , // Y
   JB_L1   , // Left  Bumper
   JB_R1   , // Right Bumper
   JB_L2   , // Left  Trigger
   JB_R2   , // Right Trigger
   JB_L3   , // Left  Stick
   JB_R3   , // Right Stick
   JB_BACK , // Back
   JB_START, // Start

   JB_LSL, // Nintendo Switch Left  SL
   JB_LSR, // Nintendo Switch Left  SR
   JB_RSL, // Nintendo Switch Right SL
   JB_RSR, // Nintendo Switch Right SR

   JB_HOME, // Home

   // alternative names
   JB_LB=JB_L1,
   JB_RB=JB_R1,
   JB_LT=JB_L2,
   JB_RT=JB_R2,
   JB_LS=JB_L3,
   JB_RS=JB_R3,

   JB_SELECT=JB_BACK , // Sony Playstation
   JB_MINUS =JB_BACK , // Nintendo Switch
   JB_PLUS  =JB_START, // Nintendo Switch

   JB_MINI_S1=JB_LSL, // Nintendo Switch Mini Side Button 1
   JB_MINI_S2=JB_LSR, // Nintendo Switch Mini Side Button 2

   JB_L=JB_X, // button located at the Left  side
   JB_R=JB_B, // button located at the Right side
   JB_U=JB_Y, // button located at the Up    side
   JB_D=JB_A, // button located at the Down  side

   JB_SQUARE  =JB_L, // button located at the Left  side
   JB_CIRCLE  =JB_R, // button located at the Right side
   JB_TRIANGLE=JB_U, // button located at the Up    side
   JB_CROSS   =JB_D, // button located at the Down  side

   // these are valid only for 'Inputs'
   JB_DPAD_LEFT =32,
   JB_DPAD_RIGHT=33,
   JB_DPAD_DOWN =34,
   JB_DPAD_UP   =35,
#if EE_PRIVATE
   JB_TOTAL,
#endif
};
#if EE_PRIVATE
inline Bool IsDPad (JOYPAD_BUTTON button) {return button>=JB_DPAD_LEFT;}
inline Bool IsDPadY(JOYPAD_BUTTON button) {return button>=JB_DPAD_DOWN;}
#endif
/******************************************************************************/
struct Vibration
{
   struct Motor
   {
      Flt intensity, // intensity, 0..1
          frequency; // frequency, Hertz
   };
   Motor motor[2];
};
/******************************************************************************/
struct Joypad // Joypad Input
{
   struct Color2
   {
      Color main, sub;

      Color2& zero()  {       main.zero();   sub.zero(); return T;}
      Bool    any ()C {return main.any () || sub.any ();}
   };
   struct Sensor
   {
      Vec    accel, // accelerometer
             gyro ; // gyroscope
      Orient orn  ; // orientation

      Sensor& reset() {accel.zero(); gyro.zero(); orn.identity(); return T;}
   };

   VecSB2 diri      , //        direction (integer version)
          diri_r    , //        direction (integer version) repeatable, will get triggered repeatedly as long as        direction is pressed
          diri_ar[2]; // analog direction (integer version) repeatable, will get triggered repeatedly as long as analog direction is pressed, [0]=left, [1]=right
   Vec2   dir       , //        direction
          dir_a  [2]; // analog direction, [0]=left, [1]=right
   Flt    trigger[2]; // trigger         , [0]=left, [1]=right

   BS_FLAG state(Int b)C {return InRange(b, _button) ?          _button[b]  : BS_NONE;} // get button 'b' state
   Bool    b    (Int b)C {return InRange(b, _button) ? ButtonOn(_button[b]) :   false;} // if  button 'b' is on
   Bool    bp   (Int b)C {return InRange(b, _button) ? ButtonPd(_button[b]) :   false;} // if  button 'b' pushed   in this frame
   Bool    br   (Int b)C {return InRange(b, _button) ? ButtonRs(_button[b]) :   false;} // if  button 'b' released in this frame
   Bool    bd   (Int b)C {return InRange(b, _button) ? ButtonDb(_button[b]) :   false;} // if  button 'b' double clicked

   void eat(Int b); // eat 'b' button from this frame so it will not be processed by the remaining codes in frame, this disables all BS_FLAG states (BS_PUSHED, BS_RELEASED, etc.) except BS_ON

   Bool supportsVibrations()C; // if supports vibrations
   Bool supportsSensors   ()C; // if supports sensors, available only if 'JoypadSensors' was enabled

           UInt           id  (     )C {return _id  ;} // get unique ID of this Joypad
          C Str&        name  (     )C {return _name;} // get Joypad name
          CChar8* buttonName  (Int b)C;                // get button name  , buttonName  (JB_A) -> "A", buttonName  (JB_DPAD_UP) -> "Up"
   static CChar8* ButtonName  (Int b);                 // get button name  , ButtonName  (JB_A) -> "A", ButtonName  (JB_DPAD_UP) -> "Up"
          CChar * buttonSymbol(Int b)C;                // get button symbol, buttonSymbol(JB_A) -> "A", buttonSymbol(JB_DPAD_UP) -> "⯅", Warning: this function might return "⯇⯈⯆⯅△□○✕", if you want to display symbols on the screen be sure to include these characters in your Font
   static CChar * ButtonSymbol(Int b);                 // get button symbol, ButtonSymbol(JB_A) -> "A", ButtonSymbol(JB_DPAD_UP) -> "⯅", Warning: this function might return "⯇⯈⯆⯅△□○✕", if you want to display symbols on the screen be sure to include these characters in your Font
            Bool        mini  (     )C {return _mini;} // if  this is a mini Joypad (a single Nintendo Switch Joy-Con Left or Right held horizontally)

   Joypad& vibration(C Vec2 &vibration                    ); // set vibrations, 'vibration.x'=left motor intensity (0..1), 'vibration.y'=right motor intensity (0..1)
   Joypad& vibration(C Vibration &left, C Vibration &right); // set vibrations

 C Vec   & accel ()C {return _sensor_right.accel;} // get accelerometer value, available only if 'supportsSensors'
 C Vec   & gyro  ()C {return _sensor_right.gyro ;} // get gyroscope     value, available only if 'supportsSensors'
 C Orient& orient()C {return _sensor_right.orn  ;} // get orientation   value, available only if 'supportsSensors'

 C Sensor& sensorLeft ()C {return _sensor_left ;} // get sensor of the left  part of the Joypad, available only if 'supportsSensors'
 C Sensor& sensorRight()C {return _sensor_right;} // get sensor of the right part of the Joypad, available only if 'supportsSensors'

 C Color2& colorLeft ()C {return _color_left ;} // get color of the left  part of the Joypad (TRANSPARENT if unknown)
 C Color2& colorRight()C {return _color_right;} // get color of the right part of the Joypad (TRANSPARENT if unknown)

#if EE_PRIVATE
   // manage
   void acquire   (Bool on);
   void setDiri   (Int x, Int y);
   void setTrigger(Int index, Flt value);
   void getState  ();
   void update    ();
   void clear     ();
   void zero      ();
   void push      (Byte b);
   void release   (Byte b);
   void sensors   (Bool calculate);
   void setInfo   (U16 vendor_id, U16 product_id);
#if IOS
   void changed   (GCControllerElement *element);
#endif
#endif

#if !EE_PRIVATE
private:
#endif
   BS_FLAG _button[32];
   Bool    _mini=false;
   Bool    _connected=false;
#if JP_GAMEPAD_INPUT
   Bool    _vibrations=false;
#endif
#if JP_GAMEPAD_INPUT || JP_X_INPUT || JP_DIRECT_INPUT
   Bool    _state_index=false;
#endif
   Byte    _joypad_index;
   Byte    _name_type=0;
#define JOYPAD_BUTTON_REMAP (JP_DIRECT_INPUT || JP_GAMEPAD_INPUT || MAC || ANDROID)
#if     JOYPAD_BUTTON_REMAP
   Byte    _remap[32];
#endif
#if JP_GAMEPAD_INPUT || ANDROID
   Byte    _axis_stick_r_x=0xFF, _axis_stick_r_y=0xFF, _axis_trigger_l=0xFF, _axis_trigger_r=0xFF;
#endif
#if JP_DIRECT_INPUT
   Byte    _axis_stick_r_x=0, _axis_stick_r_y=0;
#endif
#if JP_X_INPUT
   Byte    _xinput=255;
#endif
#define JOYPAD_VENDOR_PRODUCT_ID (JP_DIRECT_INPUT && JP_GAMEPAD_INPUT) // needed only if using both JP_DIRECT_INPUT and JP_GAMEPAD_INPUT
#if     JOYPAD_VENDOR_PRODUCT_ID
   U16     _vendor_id=0, _product_id=0;
#endif
#if JP_GAMEPAD_INPUT
   Int     _buttons=0, _switches=0, _axes=0;
#endif
   UInt    _id=0;
#if SWITCH
   typedef UInt Buttons;
   UInt    _vibration_handle[2], _sensor_handle[2];
   Buttons _state_buttons=0;
   Long    _sampling_number=-1;
#endif
   Flt     _last_t[32], _dir_t, _dir_at[2];
   Color2  _color_left, _color_right;
#if ANDROID
   Vec2    _axis_trigger_mad[2]; // x=mul, y=add, [0]=l, [1]=r
#endif
   Sensor  _sensor_left, _sensor_right;
   Str     _name;
#if JP_DIRECT_INPUT
#if EE_PRIVATE
   IDirectInputDevice8 *_dinput=null;
#else
   Ptr     _dinput=null;
#endif
#endif
#if JP_GAMEPAD_INPUT
#if EE_PRIVATE
   #if WINDOWS_OLD
      Microsoft::WRL::ComPtr<ABI::Windows::Gaming::Input::IGamepad          > _gamepad;
      Microsoft::WRL::ComPtr<ABI::Windows::Gaming::Input::IRawGameController> _raw_game_controller;
   #else
      Windows::Gaming::Input::Gamepad           ^_gamepad;
      Windows::Gaming::Input::RawGameController ^_raw_game_controller;
   #endif
#else
   Ptr     _gamepad=null, _raw_game_controller=null;
#endif
#endif
#if MAC
   struct Elm
   {
      enum TYPE : Byte
      {
         PAD   ,
         BUTTON,
         AXIS  ,
      };
      TYPE               type;
      Byte               index;
   #if EE_PRIVATE
      IOHIDElementCookie cookie;
   #else
      UInt               cookie;
   #endif
      Int                avg, max; // button_on=(val>avg);
      Flt                mul, add;

   #if EE_PRIVATE
      void setPad   (C IOHIDElementCookie &cookie                     , Int max) {T.type=PAD   ; T.cookie=cookie; T.max  =max+1; T.mul=-PI2/T.max; T.add=PI_2;}
      void setButton(C IOHIDElementCookie &cookie, Byte index, Int min, Int max) {T.type=BUTTON; T.cookie=cookie; T.index=index; T.avg=(min+max)/2;}
      void setAxis  (C IOHIDElementCookie &cookie, Byte index, Int min, Int max) {T.type=AXIS  ; T.cookie=cookie; T.index=index; T.mul=2.0f/(max-min); T.add=-1-min*T.mul; if(index==1 || index==3){CHS(mul); CHS(add);}} // change sign for vertical
   #endif
   };

   Mems<Elm> _elms;
   Ptr       _device=null;
#elif IOS
   struct Elm
   {
      enum TYPE : Byte
      {
         PAD    ,
         BUTTON ,
         TRIGGER,
         AXIS   ,
      };
      TYPE                 type;
      Byte                 index;
   #if EE_PRIVATE
      GCControllerElement *element;
   #else
      Ptr                  element;
   #endif
   #if EE_PRIVATE
      void set(TYPE type, Byte index, GCControllerElement *element) {T.type=type; T.index=index; T.element=element;}
   #endif
   };
   Mems<Elm> _elms;
#if EE_PRIVATE
   void addPad    (GCControllerElement *element);
   void addButton (GCControllerElement *element, JOYPAD_BUTTON button);
   void addTrigger(GCControllerElement *element, Byte          index );
   void addAxis   (GCControllerElement *element, Byte          index );
#endif
#if EE_PRIVATE
   GCExtendedGamepad *_gamepad=null;
#else
   Ptr       _gamepad=null;
#endif
#endif

   struct State
   {
   #if (JP_GAMEPAD_INPUT && WINDOWS_OLD) || JP_X_INPUT || JP_DIRECT_INPUT
      union
      {
         struct Data
         {
            UInt data[(32 + 4 + 6*8)/4]; // use UInt to force alignment
         }data;

      #if EE_PRIVATE
      #if JP_GAMEPAD_INPUT && WINDOWS_OLD
         ABI::Windows::Gaming::Input::GamepadReading gamepad;   ASSERT(SIZE(gamepad)<=SIZE(data));

         struct
         {
            boolean                                                   button[ELMS(_remap)];
            ABI::Windows::Gaming::Input::GameControllerSwitchPosition Switch[1];
            DOUBLE                                                    axis  [6];
         }raw_game_controller;   ASSERT(SIZE(raw_game_controller)<=SIZE(data));
      #endif
      #if JP_X_INPUT
         XINPUT_STATE xinput;   ASSERT(SIZE(xinput)<=SIZE(data));
      #endif
      #if JP_DIRECT_INPUT
         DIJOYSTATE dinput;   ASSERT(SIZE(dinput)<=SIZE(data));
      #endif
      #endif
      };
   #endif

   #if JP_GAMEPAD_INPUT && WINDOWS_NEW
   #if EE_PRIVATE
      Platform::Array<bool                                                > ^button;
      Platform::Array<Windows::Gaming::Input::GameControllerSwitchPosition> ^Switch;
      Platform::Array<double                                              > ^axis;
   #pragma pack(push)
   #pragma pack() // default packing required for storing WinRT classes not as ^ pointers, without this 'gamepad' members were getting corrupt values
      Windows::Gaming::Input::GamepadReading gamepad;   ASSERT(SIZE(gamepad)==SIZE(UInt)*16); // use UInt for alignment
   #pragma pack(pop)
   #else
      Ptr  button=null, Switch=null, axis=null;
      UInt gamepad[16];
   #endif
   #endif
   }_state[2];

  ~Joypad();
   Joypad();

   NO_COPY_CONSTRUCTOR(Joypad);
};
/******************************************************************************/
struct JoypadsClass // Container for Joypads
{
   Int     elms      (       )C {return _data.elms() ;} // number of Joypads
   Joypad& operator[](Int  i )  {return _data     [i];} // get i-th Joypad
   Joypad* addr      (Int  i )  {return _data.addr(i);} // get i-th Joypad      , null on fail
   Joypad* find      (UInt id);                         // find Joypad by its ID, null on fail

   void swapOrder(Int i  , Int j        ); // swap order of 'i' and 'j' joypads
   void moveElm  (Int elm, Int new_index); // move 'elm' joypad to new position located at 'new_index'

#if EE_PRIVATE
   void clear  ();
   void remove (Int     i     ); // remove i-th Joypad
   void remove (Joypad *joypad); // remove joypad
   void update ();
   void acquire(Bool on);
   void list   ();
#endif

#if !EE_PRIVATE
private:
#endif
   MemtN<Joypad, 4> _data;
}extern
   Joypads;
/******************************************************************************/
void ApplyDeadZone(Vec2 &v, Flt dead_zone); // apply dead zone to analog direction vector, 'dead_zone'=0..1, this can be optionally called per-frame for 'Joypad.dir_a' vectors

void JoypadSensors(Bool calculate); // if want Joypad sensors to be calculated (accelerometer, gyroscope, orientation)
Bool JoypadSensors();               // if want Joypad sensors to be calculated (accelerometer, gyroscope, orientation)

void ConfigureJoypads(Int min_players, Int max_players, C CMemPtr<Str> &player_names=null, C CMemPtr<Color> &player_colors=null); // open OS joypad configuration screen, 'min_players'=minimum number of players required, 'max_players'=maximum number of players allowed, 'player_names'=names for each player (optional), 'player_colors'=colors for each player (optional), supported only on Nintendo Switch

inline Int Elms(C JoypadsClass &jps) {return jps.elms();}
#if EE_PRIVATE
extern Bool     JoypadLayoutName;
extern SyncLock JoypadLock;

Joypad& GetJoypad  (UInt id, Bool &added);
UInt    NewJoypadID(UInt id); // generate a Joypad ID based on 'id' that's not yet used by any other existing Joypad

void ListJoypads();
void InitJoypads();
void ShutJoypads();
#endif
/******************************************************************************/
