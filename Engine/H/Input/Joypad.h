/******************************************************************************

   Use 'Joypads' container to access Joypads input.

/******************************************************************************/
enum JOYPAD_BUTTON // button indexes as defined for XInput/Xbox/NintendoSwitch controllers
{
   JB_A     , // A
   JB_B     , // B
   JB_X     , // X
   JB_Y     , // Y
   JB_L1    , // Left  Shoulder
   JB_R1    , // Right Shoulder
   JB_L2    , // Left  Trigger
   JB_R2    , // Right Trigger
   JB_LTHUMB, // Left  Thumb
   JB_RTHUMB, // Right Thumb
   JB_BACK  , // Back
   JB_START , // Start
   JB_NUM   , // number of buttons for XInput/Xbox controllers

   JB_PADDLE1=JB_NUM, // Paddle 1
   JB_PADDLE2       , // Paddle 2
   JB_PADDLE3       , // Paddle 3
   JB_PADDLE4       , // Paddle 4
   JB_UWP_NUM       , // number of buttons for WindowsUWP controllers

   JB_LSL=JB_NUM  , // Nintendo Switch Left  SL
   JB_LSR         , // Nintendo Switch Left  SR
   JB_RSL         , // Nintendo Switch Right SL
   JB_RSR         , // Nintendo Switch Right SR
   JB_NINTENDO_NUM, // number of buttons for Nintendo Switch controllers

   // alternative names
   JB_SELECT=JB_BACK , // Sony Playstation
   JB_MINUS =JB_BACK , // Nintendo Switch
   JB_PLUS  =JB_START, // Nintendo Switch

   JB_MINI_S1=JB_LSL, // Nintendo Switch Mini Side Button 1
   JB_MINI_S2=JB_LSR, // Nintendo Switch Mini Side Button 2

   // Left/Right/Up/Down
#if SWITCH
   JB_L=JB_Y, // button located at the Left  side on Nintendo Switch
   JB_R=JB_A, // button located at the Right side on Nintendo Switch
   JB_U=JB_X, // button located at the Up    side on Nintendo Switch
   JB_D=JB_B, // button located at the Down  side on Nintendo Switch
#else
   JB_L=JB_X, // button located at the Left  side
   JB_R=JB_B, // button located at the Right side
   JB_U=JB_Y, // button located at the Up    side
   JB_D=JB_A, // button located at the Down  side
#endif

   // these are valid only for 'Inputs'
   JB_DPAD_LEFT =32,
   JB_DPAD_RIGHT=33,
   JB_DPAD_DOWN =34,
   JB_DPAD_UP   =35,
};
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

            UInt          id(     )C {return _id  ;} // get unique ID of this Joypad
           C Str&       name(     )C {return _name;} // get Joypad name
          CChar8* buttonName(Int b)C;                // get button name, buttonName(JB_A) -> "A", buttonName(JB_B) -> "B", ..
   static CChar8* ButtonName(Int b);                 // get button name, ButtonName(JB_A) -> "A", ButtonName(JB_B) -> "B", ..
            Bool        mini(     )C;                // if  this is a mini Joypad (a single Nintendo Switch Joy-Con Left or Right held horizontally)

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
   void acquire (Bool on);
   void update  (C Byte *on, Int elms);
   void setDiri (Int x, Int y);
   void updateOK();
   void update  ();
   void clear   ();
   void zero    ();
   void push    (Byte b);
   void release (Byte b);
   Int  index   ()C;
   void sensors (Bool calculate);
#endif

#if !EE_PRIVATE
private:
#endif
   BS_FLAG _button[32];
#if JP_DIRECT_INPUT
   Byte    _offset_x=0, _offset_y=0;
#endif
#if JP_X_INPUT
   Byte    _xinput=0xFF;
#endif
#if JP_GAMEPAD_INPUT
   Bool    _vibrations=false;
#endif
#if SWITCH
   Bool    _mini=false;
#endif
   Bool    _connected=false;
#define JOYPAD_VENDOR_PRODUCT_ID (JP_DIRECT_INPUT && JP_GAMEPAD_INPUT) // needed only if using both JP_DIRECT_INPUT and JP_GAMEPAD_INPUT
#if     JOYPAD_VENDOR_PRODUCT_ID
   U16     _vendor_id=0, _product_id=0;
#endif
#if JP_GAMEPAD_INPUT
   Int     _buttons=0, _switches=0, _axes=0;
#endif
   UInt    _id=0;
#if SWITCH
   UInt    _vibration_handle[2], _sensor_handle[2];
#endif
   Flt     _last_t[32], _dir_t, _dir_at[2];
   Color2  _color_left, _color_right;
   Sensor  _sensor_left, _sensor_right;
   Str     _name;
#if JP_DIRECT_INPUT
#if EE_PRIVATE
   IDirectInputDevice8 *_device=null;
#else
   Ptr     _device=null;
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

#if WINDOWS_NEW
#if EE_PRIVATE
   Platform::Array<bool                                                > ^_array_button;
   Platform::Array<Windows::Gaming::Input::GameControllerSwitchPosition> ^_array_switch;
   Platform::Array<double                                              > ^_array_axis;
#else
   Ptr     _array_button=null, _array_switch=null, _array_axis=null;
#endif
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
      Int                index;
   #if EE_PRIVATE
      IOHIDElementCookie cookie;
   #else
      UInt               cookie;
   #endif
      Int                avg, max; // button_on=(val>avg);
      Flt                mul, add;

   #if EE_PRIVATE
      void setPad   (C IOHIDElementCookie &cookie                    , Int max) {T.type=PAD   ; T.cookie=cookie; T.max  =max+1; T.mul=-PI2/T.max; T.add=PI_2;}
      void setButton(C IOHIDElementCookie &cookie, Int index, Int min, Int max) {T.type=BUTTON; T.cookie=cookie; T.index=index; T.avg=(min+max)/2;}
      void setAxis  (C IOHIDElementCookie &cookie, Int index, Int min, Int max) {T.type=AXIS  ; T.cookie=cookie; T.index=index; T.mul=2.0f/(max-min); T.add=-1-min*T.mul; if(index&1){CHS(mul); CHS(add);}} // change sign for vertical
   #endif
   };

   Mems<Elm> _elms;
   Byte      _mac_button[32];
   Ptr       _device=null;
#endif

   static CChar8 *_button_name[];

  ~Joypad();
   Joypad();

   NO_COPY_CONSTRUCTOR(Joypad);
};
extern MemtN<Joypad, 4> Joypads;
/******************************************************************************/
void ApplyDeadZone(Vec2 &v, Flt dead_zone); // apply dead zone to analog direction vector, 'dead_zone'=0..1, this can be optionally called per-frame for 'Joypad.dir_a' vectors

void JoypadSensors(Bool calculate); // if want Joypad sensors to be calculated (accelerometer, gyroscope, orientation)
Bool JoypadSensors();               // if want Joypad sensors to be calculated (accelerometer, gyroscope, orientation)

void ConfigureJoypads(Int min_players, Int max_players, C CMemPtr<Str> &player_names=null, C CMemPtr<Color> &player_colors=null); // open OS joypad configuration screen, 'min_players'=minimum number of players required, 'max_players'=maximum number of players allowed, 'player_names'=names for each player (optional), 'player_colors'=colors for each player (optional), supported only on Nintendo Switch

Joypad* FindJoypad(UInt id); // find Joypad in 'Joypads' container according to its 'id', null on fail
#if EE_PRIVATE
Joypad& GetJoypad  (UInt id, Bool &added);
UInt    NewJoypadID(UInt id); // generate a Joypad ID based on 'id' that's not yet used by any other existing Joypad

void ListJoypads();
void InitJoypads();
void ShutJoypads();
#endif
/******************************************************************************/
