/******************************************************************************

   Joypads are not sorted by their ID because:
      -we can manually change their order using "Joypads.swapOrder", "Joypads.moveElm"
      -we want new joypads to be added to the end of the list

/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************/
#define JOYPAD_THREAD (JP_GAMEPAD_INPUT || JP_X_INPUT || JP_DIRECT_INPUT)
#define JOYPAD_THREAD_SLEEP 5
/******************************************************************************/
Bool JoypadLayoutName; // if set layout to match name, if false then direction, default=false (direction)
static Bool CalculateJoypadSensors;

static CChar8 *JoypadButtonName[JP_NAMES][JB_TOTAL];
static CChar *JoypadButtonSymbol[JP_NAMES][JB_TOTAL];

JoypadsClass Joypads;
/******************************************************************************/
#if JOYPAD_THREAD
static Memc<Input> ThreadInputs;
SyncLock JoypadLock;
static Thread JoypadThread;
static SyncEvent JoypadEvent;
#if WINDOWS_OLD
static Int TimerRes;
#endif
static Bool JoypadThreadFunc(Thread &thread) {
#if DEBUG && 0
    static Flt t;
    Flt t1 = Time.curTime();
    LogName(S);
    LogN(t1 - t);
    t = t1;
#endif
    {
        SyncLocker lock(JoypadLock);
        REPAO(Joypads).getState();
    }
    JoypadEvent.wait(JOYPAD_THREAD_SLEEP);
    return true;
}
#endif
/******************************************************************************/
#if JP_GAMEPAD_INPUT && WINDOWS_OLD
static Microsoft::WRL::ComPtr<ABI::Windows::Gaming::Input::IGamepadStatics> GamepadStatics;
static Microsoft::WRL::ComPtr<ABI::Windows::Gaming::Input::IGamepadStatics2> GamepadStatics2;
static Microsoft::WRL::ComPtr<ABI::Windows::Gaming::Input::IRawGameControllerStatics> RawGameControllerStatics;
static EventRegistrationToken GamepadAddedToken, GamepadRemovedToken, RawGameControllerAddedToken, RawGameControllerRemovedToken;

static Int FindJoypadI(ABI::Windows::Gaming::Input::IGamepad *gamepad) {
    REPA(Joypads)
    if (Joypads[i]._gamepad.Get() == gamepad)
        return i;
    return -1;
}
static Int FindJoypadI(ABI::Windows::Gaming::Input::IRawGameController *raw_game_controller) {
    REPA(Joypads)
    if (Joypads[i]._raw_game_controller.Get() == raw_game_controller)
        return i;
    return -1;
}

struct GamepadChange {
    Bool added;
    Microsoft::WRL::ComPtr<ABI::Windows::Gaming::Input::IGamepad> gamepad;
    Microsoft::WRL::ComPtr<ABI::Windows::Gaming::Input::IRawGameController> raw_game_controller;

    void process() {
        if (added) {
            if (gamepad && FindJoypadI(gamepad.Get()) >= 0 // make sure it's not already listed
                || raw_game_controller && FindJoypadI(raw_game_controller.Get()) >= 0)
                return; // make sure it's not already listed

            Microsoft::WRL::ComPtr<ABI::Windows::Gaming::Input::IRawGameController2> raw_game_controller2;
            Microsoft::WRL::ComPtr<ABI::Windows::Gaming::Input::IGameController> game_controller;
            if (gamepad) // gamepad callback
            {
                gamepad->QueryInterface(IID_PPV_ARGS(&game_controller));
                if (game_controller) {
                    if (RawGameControllerStatics)
                        RawGameControllerStatics->FromGameController(game_controller.Get(), &raw_game_controller);
                }
            } else // raw_game_controller callback
            {
                raw_game_controller->QueryInterface(IID_PPV_ARGS(&game_controller));
                if (game_controller) {
                    if (GamepadStatics2) {
                        GamepadStatics2->FromGameController(game_controller.Get(), &gamepad);
                        if (gamepad)
                            return; // if this is a gamepad, then ignore this 'raw_game_controller' callback, as it will be processed using 'gamepad' callback, to avoid having the same gamepad listed twice
                    }
                }
            }

            U16 vendor_id = 0, product_id = 0;
            UInt joypad_id = 0;
            if (raw_game_controller) {
                raw_game_controller->get_HardwareVendorId(&vendor_id);
                raw_game_controller->get_HardwareProductId(&product_id);
#if JP_DIRECT_INPUT           // if we use DirectInput then have to remove all DirectInput joypads with same vendor/product ID as they will be processed using this API instead
                REPA(Joypads) // go from back because we remove
                {
                    Joypad &jp = Joypads[i];
                    if (jp._vendor_id == vendor_id && jp._product_id == product_id && jp._dinput)
                        Joypads.remove(i); // remove all (not just one)
                }
#endif
                raw_game_controller.As(&raw_game_controller2);
                if (raw_game_controller2) {
                    HSTRING id = null;
                    raw_game_controller2->get_NonRoamableId(&id);
                    C wchar_t *controller_id = WindowsGetStringRawBuffer(id, null);
                    joypad_id = xxHash64_32Mem(controller_id, SIZE(*controller_id) * Length(controller_id));
                }
            }

#if JOYPAD_THREAD
            SyncLocker lock(JoypadLock);
#endif
            joypad_id = NewJoypadID(joypad_id); // make sure it's not used yet !! set this before creating new 'Joypad' !!
            Bool added;
            Joypad &joypad = GetJoypad(joypad_id, added);
            if (added) {
                joypad._gamepad = gamepad;
                joypad._raw_game_controller = raw_game_controller;

                if (raw_game_controller) {
                    raw_game_controller->get_ButtonCount(&joypad._buttons);
                    MIN(joypad._buttons, Elms(joypad._state[0].raw_game_controller.button));
                    raw_game_controller->get_SwitchCount(&joypad._switches);
                    MIN(joypad._switches, Elms(joypad._state[0].raw_game_controller.Switch));
                    raw_game_controller->get_AxisCount(&joypad._axes);
                    MIN(joypad._axes, Elms(joypad._state[0].raw_game_controller.axis));
                    /* were empty on Logitech F310 in DirectInput mode, Nintendo JoyCons
                    FREP(buttons)
                    {
                       ABI::Windows::Gaming::Input::GameControllerButtonLabel label=ABI::Windows::Gaming::Input::GameControllerButtonLabel_None;
                      _raw_game_controller->GetButtonLabel(i, &label);
                    }*/
                    Microsoft::WRL::ComPtr<ABI::Windows::Foundation::Collections::IVectorView<ABI::Windows::Gaming::Input::ForceFeedback::ForceFeedbackMotor *>> motors;
                    raw_game_controller->get_ForceFeedbackMotors(&motors);
                    if (motors) {
                        unsigned motors_count = 0;
                        motors->get_Size(&motors_count);
                        joypad._vibrations = (motors_count > 0);
                    }
                }
                if (raw_game_controller2) {
                    HSTRING display_name = null;
                    raw_game_controller2->get_DisplayName(&display_name);
                    joypad._name = WindowsGetStringRawBuffer(display_name, null);
                }
                joypad.setInfo(vendor_id, product_id);

                if (auto func = App.joypad_changed)
                    func(); // call at the end
            }
        } else {
            if (gamepad)
                Joypads.remove(FindJoypadI(gamepad.Get()));
            if (raw_game_controller)
                Joypads.remove(FindJoypadI(raw_game_controller.Get()));
        }
    }
};
static MemcThreadSafe<GamepadChange> GamepadChanges;

static void GamepadChanged() {
    MemcThreadSafeLock lock(GamepadChanges);
    FREPA(GamepadChanges)
    GamepadChanges.lockedElm(i).process();
    GamepadChanges.lockedClear(); // process in order
}
static void GamepadChanged(Bool added, ABI::Windows::Gaming::Input::IGamepad *gamepad, ABI::Windows::Gaming::Input::IRawGameController *raw_game_controller) {
    {
        MemcThreadSafeLock lock(GamepadChanges);
        GamepadChange &gpc = GamepadChanges.lockedNew();
        gpc.added = added;
        gpc.gamepad = gamepad;
        gpc.raw_game_controller = raw_game_controller;
    }
    App.addFuncCall(GamepadChanged);
}

// !! THESE ARE CALLED ON SECONDARY THREADS !!
static struct GamepadAddedClass : Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, __FIEventHandler_1_Windows__CGaming__CInput__CGamepad> {
    virtual HRESULT Invoke(IInspectable *, ABI::Windows::Gaming::Input::IGamepad *gamepad) override {
        GamepadChanged(true, gamepad, null);
        return S_OK;
    }
} GamepadAdded;
static struct GamepadRemovedClass : Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, __FIEventHandler_1_Windows__CGaming__CInput__CGamepad> {
    virtual HRESULT Invoke(IInspectable *, ABI::Windows::Gaming::Input::IGamepad *gamepad) override {
        GamepadChanged(false, gamepad, null);
        return S_OK;
    }
} GamepadRemoved;

static struct RawGameControllerAddedClass : Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, __FIEventHandler_1_Windows__CGaming__CInput__CRawGameController> {
    virtual HRESULT Invoke(IInspectable *, ABI::Windows::Gaming::Input::IRawGameController *raw_game_controller) override {
        GamepadChanged(true, null, raw_game_controller);
        return S_OK;
    }
} RawGameControllerAdded;
static struct RawGameControllerRemovedClass : Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, __FIEventHandler_1_Windows__CGaming__CInput__CRawGameController> {
    virtual HRESULT Invoke(IInspectable *, ABI::Windows::Gaming::Input::IRawGameController *raw_game_controller) override {
        GamepadChanged(false, null, raw_game_controller);
        return S_OK;
    }
} RawGameControllerRemoved;

#elif MAC
ASSERT(MEMBER_SIZE(Joypad::Elm, cookie) == SIZE(UInt)); // because it's stored as UInt for EE_PRIVATE
static IOHIDManagerRef HidManager;
static UInt JoypadsID;
/******************************************************************************/
static Int Compare(C Joypad::Elm &a, C Joypad::Elm &b) { return Compare(UIntPtr(a.cookie), UIntPtr(b.cookie)); }
static Int Compare(C Joypad::Elm &a, C IOHIDElementCookie &b) { return Compare(UIntPtr(a.cookie), UIntPtr(b)); }

static NSMutableDictionary *JoypadCriteria(UInt32 inUsagePage, UInt32 inUsage) {
    NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
    [dict setObject:[NSNumber numberWithInt:inUsagePage] forKey:(NSString *)CFSTR(kIOHIDDeviceUsagePageKey)];
    [dict setObject:[NSNumber numberWithInt:inUsage] forKey:(NSString *)CFSTR(kIOHIDDeviceUsageKey)];
    return dict;
}
static void JoypadAdded(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef device) // this is called on the main thread
{
    Memt<Joypad::Elm> elms;
    NSArray *elements = (NSArray *)IOHIDDeviceCopyMatchingElements(device, null, kIOHIDOptionsTypeNone);
    Int axes = 2;
    FREP([elements count]) // process in order
    {
        IOHIDElementRef element = (IOHIDElementRef)[elements objectAtIndex:i];
        Int type = IOHIDElementGetType(element),
            usage = IOHIDElementGetUsage(element),
            page = IOHIDElementGetUsagePage(element),
            min = IOHIDElementGetPhysicalMin(element),
            max = IOHIDElementGetPhysicalMax(element),
            lmin = IOHIDElementGetLogicalMin(element),
            lmax = IOHIDElementGetLogicalMax(element);
        IOHIDElementCookie cookie = IOHIDElementGetCookie(element);
        // CFStringRef      elm_name=IOHIDElementGetName  (element); NSLog(@"%@", (NSString*)elm_name);

        switch (type) {
            // case kIOHIDElementTypeInput_Axis  : break;
        case kIOHIDElementTypeInput_Button:
            usage--;
            if (InRange(usage, MEMBER(Joypad, _remap)))
                elms.New().setButton(cookie, usage, min, max);
            break;
        case kIOHIDElementTypeInput_Misc:
            if (usage >= 0x30 && usage < 0x32)
                elms.New().setAxis(cookie, usage - 0x30, min, max);
            else // Left  XY (0x30, 0x31) are always for Left XY, process it like this because Samsung EI-GP20 reports these 2 times!
                if (usage >= 0x32 && usage < 0x36)
                    elms.New().setAxis(cookie, axes++, min, max);
                else // Right XY
                    if (usage == 0x39)
                        elms.New().setPad(cookie, lmax);
            break; // 0x39 is always DPad
        }
    }

    if (elms.elms()) {
        elms.sort(Compare); // sort so 'binaryFind' can be used later

        NSString *name = (NSString *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey)); // do not release this !!
                                                                                              // NSString *serial= (NSString*)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDSerialNumberKey)); // do not release this ? this was null on "Logitech Rumblepad 2"
        Int vendor_id = [(NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey)) intValue];
        Int product_id = [(NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey)) intValue];

#if JOYPAD_THREAD
        SyncLocker lock(JoypadLock);
#endif
        Bool added;
        Joypad &jp = GetJoypad(JoypadsID++, added);
        if (added) {
            jp._name = name;
            jp._device = device;
            jp.setInfo(vendor_id, product_id);
            REPA(elms) {
                Joypad::Elm &elm = elms[i];
                if (elm.type == Joypad::Elm::BUTTON) {
                    Byte remap = jp._remap[elm.index];
                    if (InRange(remap, jp._button))
                        elm.index = remap;
                    else
                        elms.remove(i, true);
                }
            }
            jp._elms = elms;

            if (auto func = App.joypad_changed)
                func(); // call at the end
        }
    }
}
static void JoypadRemoved(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef device) // this is called on the main thread
{
    REPA(Joypads)
    if (Joypads[i]._device == device) {
        Joypads.remove(i);
        break;
    }
}
static void JoypadAction(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef value) // this is called on the main thread
{
    IOHIDElementRef element = IOHIDValueGetElement(value);
    IOHIDDeviceRef device = IOHIDElementGetDevice(element); // or IOHIDQueueGetDevice((IOHIDQueueRef)inSender);
    REPA(Joypads) {
        Joypad &jp = Joypads[i];
        if (jp._device == device) {
            if (C Joypad::Elm *elm = jp._elms.binaryFind(IOHIDElementGetCookie(element), Compare)) {
                Int val = IOHIDValueGetIntegerValue(value);
                switch (elm->type) {
                case Joypad::Elm::PAD: {
                    if (InRange(val, elm->max)) {
                        CosSin(jp.dir.x, jp.dir.y, val * elm->mul + elm->add);
                        jp.setDiri(Round(jp.dir.x), Round(jp.dir.y));
                    } else {
                        jp.dir.zero();
                        jp.setDiri(0, 0);
                    }
                } break;

                case Joypad::Elm::BUTTON: {
                    if (val > elm->avg)
                        jp.push(elm->index);
                    else
                        jp.release(elm->index);
                } break;

                case Joypad::Elm::AXIS: {
                    switch (elm->index) {
                    case 0:
                        jp.dir_a[0].x = val * elm->mul + elm->add;
                        break;
                    case 1:
                        jp.dir_a[0].y = val * elm->mul + elm->add;
                        break;
                    case 2:
                        jp.dir_a[1].x = val * elm->mul + elm->add;
                        break;
                    case 3:
                        jp.dir_a[1].y = val * elm->mul + elm->add;
                        break;
                    }
                } break;
                }
            }
            break;
        }
    }
}
#endif
/******************************************************************************/
static void RemovingJoypad(Memc<Input> &inputs, Int device) {
    REPA(inputs) // go from the end because we're removing elements
    {
        Input &input = inputs[i];
        if (input.type == INPUT_JOYPAD) {
            if (input.device > device)
                input.device--;
            else // adjust the index
                if (input.device == device)
                    inputs.remove(i, true); // if this input is for joypad being deleted, then delete the input and keep order
        }
    }
}
static void AddingJoypad(Memc<Input> &inputs, Int device) {
    REPA(inputs) {
        Input &input = inputs[i];
        if (input.type == INPUT_JOYPAD && input.device >= device)
            input.device++;
    }
}
/******************************************************************************/
Joypad::~Joypad() {
#if JP_DIRECT_INPUT
    if (_dinput) {
        _dinput->Unacquire();
        _dinput->Release();
        _dinput = null;
    }
#endif

    // adjust 'Inputs', because it holds a 'device' index, so have to adjust those indexes
    if (InRange(_joypad_index, Joypads)) {
        RemovingJoypad(Inputs, _joypad_index);
#if JOYPAD_THREAD
        RemovingJoypad(ThreadInputs, _joypad_index); // lock not needed because this is called only under lock
#endif
        REPA(Joypads) {
            Joypad &jp = Joypads[i];
            if (jp._joypad_index > _joypad_index)
                jp._joypad_index--;
        } // don't modify self, only fix those that will be kept
    }
}
Joypad::Joypad() {
    ASSERT(ELMS(_last_t) == ELMS(_button));

#if SWITCH
    Zero(_vibration_handle);
    Zero(_sensor_handle);
#endif

    _color_left.zero();
    _color_right.zero();

    zero();

    // adjust 'Inputs', because it holds a 'device' index, so have to adjust those indexes
    if (Joypads._data.contains(this)) // in 'Joypads'
    {
        REPAO(Joypads)._joypad_index = i;       // recalculate indexes for all joypads
        if (_joypad_index < Joypads.elms() - 1) // this is not the last one, then it means we've just moved some existing joypads to the right
        {
            AddingJoypad(Inputs, _joypad_index);
#if JOYPAD_THREAD
            AddingJoypad(ThreadInputs, _joypad_index); // lock not needed because this is called only under lock
#endif
        }
#if JOYPAD_THREAD
        if ((App.flag & APP_JOYPAD_THREAD) && !JoypadThread.active()) {
#if WINDOWS_OLD // there's no option to increase precision for UWP
            TIMECAPS tc;
            if (timeGetDevCaps(&tc, SIZE(tc)) == MMSYSERR_NOERROR)
                timeBeginPeriod(TimerRes = Mid(JOYPAD_THREAD_SLEEP, tc.wPeriodMin, tc.wPeriodMax)); // need to enable high precision timers because default accuracy is around 16 ms
#endif
            JoypadThread.create(JoypadThreadFunc, null, 2, false, "EE.Joypad");
        }
#endif
    } else
        _joypad_index = 255;
}
CChar8 *Joypad::buttonName(Int b) C { return InRange(b, JB_TOTAL) ? JoypadButtonName[_name_type][b] : null; }
CChar *Joypad::buttonSymbol(Int b) C { return InRange(b, JB_TOTAL) ? JoypadButtonSymbol[_name_type][b] : null; }
CChar8 *Joypad::ButtonName(Int b) {
    if (InRange(b, JB_TOTAL))
        return
#if NINTENDO
            JoypadButtonName[JP_NAME_NINTENDO][b];
#elif SONY
            JoypadButtonName[JP_NAME_SONY][b];
#else
            JoypadButtonName[JP_NAME][b];
#endif
    return null;
}
CChar *Joypad::ButtonSymbol(Int b) {
    if (InRange(b, JB_TOTAL))
        return
#if NINTENDO
            JoypadButtonSymbol[JP_NAME_NINTENDO][b];
#elif SONY
            JoypadButtonSymbol[JP_NAME_SONY][b];
#else
            JoypadButtonSymbol[JP_NAME][b];
#endif
    return null;
}
/******************************************************************************/
Bool Joypad::supportsVibrations() C {
#if JP_GAMEPAD_INPUT
    return _vibrations;
#elif JP_X_INPUT
    return _xinput != 255;
#elif SWITCH
    return _vibration_handle[0]; // || _vibration_handle[1]; check only first because second will be available only if first is
#endif
    return false;
}
Bool Joypad::supportsSensors() C {
#if SWITCH
    return _sensor_handle[0]; // || _sensor_handle[1]; check only first because second will be available only if first is
#else
    return false;
#endif
}
/******************************************************************************/
#if !SWITCH
Joypad &Joypad::vibration(C Vec2 &vibration) {
    const Bool mono = true; // MS Xbox Controllers have different motors on left/right (360, Xbox One, Elite 2)
#if JP_GAMEPAD_INPUT
    if (_gamepad) {
#if WINDOWS_OLD
        ABI::Windows::Gaming::Input::GamepadVibration v;
#else
        Windows::Gaming::Input::GamepadVibration v;
#endif
        if (mono) {
            v.LeftMotor =
                v.RightMotor = Sat(vibration.max());
        } else {
            v.LeftMotor = Sat(vibration.x);
            v.RightMotor = Sat(vibration.y);
        }
        v.LeftTrigger = v.RightTrigger = 0;
#if WINDOWS_OLD
        _gamepad->put_Vibration(v);
#else
        _gamepad->Vibration = v;
#endif
    }
#elif JP_X_INPUT
    if (_xinput != 255) {
        XINPUT_VIBRATION xvibration;
        if (mono) {
            xvibration.wLeftMotorSpeed =
                xvibration.wRightMotorSpeed = RoundU(Sat(vibration.max()) * 0xFFFF);
        } else {
            xvibration.wLeftMotorSpeed = RoundU(Sat(vibration.x) * 0xFFFF);
            xvibration.wRightMotorSpeed = RoundU(Sat(vibration.y) * 0xFFFF);
        }
        XInputSetState(_xinput, &xvibration);
    }
#endif
    return T;
}
Joypad &Joypad::vibration(C Vibration &left, C Vibration &right) {
    return vibration(Vec2(Max(left.motor[0].intensity, left.motor[1].intensity),
                          Max(right.motor[0].intensity, right.motor[1].intensity)));
}
#endif
/******************************************************************************/
void Joypad::setInfo(U16 vendor_id, U16 product_id) {
#if JOYPAD_VENDOR_PRODUCT_ID
    _vendor_id = vendor_id;
    _product_id = product_id;
#endif
#if JP_GAMEPAD_INPUT
    if (_axes >= 4) {
        _axis_stick_r_x = 2;
        _axis_stick_r_y = 3;
    }
    if (_axes >= 6) {
        _axis_trigger_l = 4;
        _axis_trigger_r = 5;
    }
#endif
#if JOYPAD_BUTTON_REMAP
    ASSERT(ELMS(_remap) == ELMS(_button));
    switch (vendor_id) {
    case 1118: // Microsoft
    {
        switch (product_id) {
        case 2821: // Xbox Elite Wireless Controller Series 2
        {
            SetMem(_remap, 255);
            _remap[0] = JB_A;
            _remap[1] = JB_B;
            _remap[2] = JB_X;
            _remap[3] = JB_Y;
            _remap[4] = JB_L1;
            _remap[5] = JB_R1;
            _remap[6] = JB_BACK;
            _remap[7] = JB_START;
            _remap[8] = JB_L3;
            _remap[9] = JB_R3;
            /* These were reported for JP_GAMEPAD_INPUT, but not on Android
              _remap[10]=JB_DPAD_UP;
              _remap[11]=JB_DPAD_RIGHT;
              _remap[12]=JB_DPAD_DOWN;
              _remap[13]=JB_DPAD_LEFT; */
            //_axis_stick_r_x=AMOTION_EVENT_AXIS_Z; don't set because auto-detect works fine
            //_axis_stick_r_y=AMOTION_EVENT_AXIS_RZ;
            //_axis_trigger_l=AMOTION_EVENT_AXIS_BRAKE;
            //_axis_trigger_r=AMOTION_EVENT_AXIS_GAS;
        }
            return;
        }
    } break;

    case 1133: // Logitech
               /*if(product_id==49693 // F310
                 || product_id==49688 // RumblePad 2
               */
        {
            SetMem(_remap, 255);
            _remap[1] = JB_A;
            _remap[2] = JB_B;
            _remap[0] = JB_X;
            _remap[3] = JB_Y;
            _remap[4] = JB_L1;
            _remap[5] = JB_R1;
            _remap[6] = JB_L2;
            _remap[7] = JB_R2;
            _remap[10] = JB_L3;
            _remap[11] = JB_R3;
            _remap[8] = JB_BACK;
            _remap[9] = JB_START;
        }
        return;

    case 1256: // Samsung
    {
        switch (product_id) {
        case 40960: // EI-GP20
        {
            SetMem(_remap, 255);
            _remap[0] = JB_A;
            _remap[1] = JB_B;
            _remap[3] = JB_X;
            _remap[4] = JB_Y;
            _remap[6] = JB_L1;
            _remap[7] = JB_R1;
            _remap[10] = JB_BACK;
            _remap[11] = JB_START;
            _remap[15] = 254; // JB_PLAY
            //_axis_stick_r_x=AMOTION_EVENT_AXIS_RX; don't set because auto-detect works fine
            //_axis_stick_r_y=AMOTION_EVENT_AXIS_RY; don't set because auto-detect works fine
            // no triggers
        }
            return;
        }
    } break;

    case 1356: // Sony
    {
        _name_type = JP_NAME_SONY;
        // product_id 2508 DualShock 4
        // product_id 3302 DualSense
        SetMem(_remap, 255);
        _remap[1] = JB_A;
        _remap[2] = JB_B;
        _remap[0] = JB_X;
        _remap[3] = JB_Y;
        _remap[4] = JB_L1;
        _remap[5] = JB_R1;
        _remap[6] = JB_L2;
        _remap[7] = JB_R2;
        _remap[10] = JB_L3;
        _remap[11] = JB_R3;
        _remap[8] = JB_BACK;
        _remap[9] = JB_START;
        _remap[13] = 254; // JB_TOUCHSCREEN
//_axis_stick_r_x=AMOTION_EVENT_AXIS_Z ; don't set because auto-detect works fine
//_axis_stick_r_y=AMOTION_EVENT_AXIS_RZ; don't set because auto-detect works fine
//_axis_trigger_l=AMOTION_EVENT_AXIS_RX; don't set because auto-detect works fine
//_axis_trigger_r=AMOTION_EVENT_AXIS_RY; don't set because auto-detect works fine
#if JP_GAMEPAD_INPUT
        if (_axes >= 6) {
            _axis_stick_r_x = 2;
            _axis_stick_r_y = 5;
            _axis_trigger_l = 3;
            _axis_trigger_r = 4;
        }
#endif
    }
        return;

    case 1406: // Nintendo
    {
        _name_type = JP_NAME_NINTENDO;
        switch (product_id) {
        case 8198: // JoyConL
        case 8199: // JoyConR
        {
            _mini = true;
            SetMem(_remap, 255);
            if (JoypadLayoutName) { // here use Nintendo layout
                _remap[0] = JB_B;   // Down
                _remap[1] = JB_A;   // Right
                _remap[2] = JB_Y;   // Left
                _remap[3] = JB_X;   // Up
            } else {
                _remap[0] = JB_D; // Down
                _remap[1] = JB_R; // Right
                _remap[2] = JB_L; // Left
                _remap[3] = JB_U; // Up
            }
            _remap[4] = JB_L1;
            _remap[5] = JB_R1;
            if (product_id == 8198) // JoyConL
            {
                _remap[10] = JB_L3;
                _remap[8] = JB_BACK;
                _remap[13] = JB_START;
            } else // JoyConR
            {
                _remap[11] = JB_L3;
                _remap[12] = JB_BACK;
                _remap[9] = JB_START;
            }
            _remap[14] = JB_MINI_S1;
            _remap[15] = JB_MINI_S2;
        }
            return;

        case 8201: // Switch Pro Controller
        {
#if JP_GAMEPAD_INPUT // use as RawGameController and not Gamepad because driver doesn't interpret buttons correctly
            if (_raw_game_controller)
                _gamepad = null;
#endif
            SetMem(_remap, 255);
            if (JoypadLayoutName) {
                _remap[0] = JB_A; // A Right
                _remap[1] = JB_X; // X Up
                _remap[2] = JB_B; // B Down
                _remap[3] = JB_Y; // Y Left
            } else {
                _remap[0] = JB_R; // A Right
                _remap[1] = JB_U; // X Up
                _remap[2] = JB_D; // B Down
                _remap[3] = JB_L; // Y Left
            }
            _remap[4] = JB_L1;
            _remap[5] = JB_R1;
            _remap[6] = JB_L2;
            _remap[7] = JB_R2;
            _remap[8] = JB_BACK;
            _remap[9] = JB_START;
            _remap[10] = JB_L3;
            _remap[11] = JB_R3;
            _remap[12] = JB_HOME;
            _remap[13] = JB_MINI_S1; // CAPTURE
            //_axis_stick_r_x=AMOTION_EVENT_AXIS_Z ; don't set because auto-detect works fine
            //_axis_stick_r_y=AMOTION_EVENT_AXIS_RZ; don't set because auto-detect works fine
            // no triggers
        }
            return;
        }
    } break;

    case 11720: // 8BitDo
    {
        // product_id 24582 Pro2
        SetMem(_remap, 255);
        _remap[0] = JB_B;
        _remap[1] = JB_A;
        _remap[3] = JB_Y;
        _remap[4] = JB_X;
        _remap[6] = JB_L1;
        _remap[7] = JB_R1;
        _remap[8] = JB_L2;
        _remap[9] = JB_R2;
        _remap[10] = JB_BACK;
        _remap[11] = JB_START;
        _remap[12] = JB_HOME;
        _remap[13] = JB_L3;
        _remap[14] = JB_R3;
//_axis_stick_r_x=AMOTION_EVENT_AXIS_Z    ; don't set because auto-detect works fine
//_axis_stick_r_y=AMOTION_EVENT_AXIS_RZ   ; don't set because auto-detect works fine
//_axis_trigger_l=AMOTION_EVENT_AXIS_BRAKE; don't set because auto-detect works fine
//_axis_trigger_r=AMOTION_EVENT_AXIS_GAS  ; don't set because auto-detect works fine
#if JP_GAMEPAD_INPUT
        if (_axes >= 6) {
            _axis_trigger_l = 5;
            _axis_trigger_r = 4;
        }
#endif
    }
        return;
    }
    if (ANDROID)
        SetMem(_remap, 255); // on Android if mapping is unknown then set 255, so we can try to use Android mappings
    else
        REPAO(_remap) = i;
#endif
}
/******************************************************************************/
void Joypad::zero() {
    Zero(_button);
    REPAO(_last_t) = -FLT_MAX;
    _dir_t = -FLT_MAX;
    REPAO(_dir_at) = -FLT_MAX;
    REPAO(diri_ar).zero();
    diri_r.zero();
    diri.zero();
    dir.zero();
    REPAO(dir_a).zero();
    REPAO(trigger) = 0;
    _sensor_left.reset();
    _sensor_right.reset();

#if SWITCH
    _state_buttons = 0;
#endif

#if JOYPAD_THREAD
    REPA(_state) // here no need for lock, because this functions is called under lock already
    {
        auto &state = _state[i];
#if (JP_GAMEPAD_INPUT && WINDOWS_OLD) || JP_X_INPUT || JP_DIRECT_INPUT
        Zero(state.data);
#endif
#if JP_DIRECT_INPUT
        if (_dinput)
            state.dinput.rgdwPOV[0] = UINT_MAX; // set initial DPAD to centered
#endif
#if JP_GAMEPAD_INPUT && WINDOWS_NEW
        if (_gamepad) {
            state.gamepad.Buttons = Windows::Gaming::Input::GamepadButtons::None;
            state.gamepad.LeftThumbstickX = 0;
            state.gamepad.RightThumbstickX = 0;
            state.gamepad.LeftThumbstickY = 0;
            state.gamepad.RightThumbstickY = 0;
            state.gamepad.LeftTrigger = state.gamepad.RightTrigger = 0;
        } else if (_raw_game_controller) {
            if (state.button)
                REP(state.button->Length)
            state.button->Data[i] = false;
            if (state.Switch)
                REP(state.Switch->Length)
            state.Switch->Data[i] = Windows::Gaming::Input::GameControllerSwitchPosition::Center;
            if (state.axis)
                REP(state.axis->Length)
            state.axis->Data[i] = ((i < 4) ? 0.5f : 0);
        }
#endif
    }
#endif
}
void Joypad::clear() // called at end of frame
{
    REPAO(_button) &= ~BS_NOT_ON;
    diri_r.zero();
    REPAO(diri_ar).zero();
}
static void UpdateDirRep1(VecSB2 &diri_r, C VecSB2 &diri, Flt &time, Flt first_time, Flt repeat_time) // skips adding for first time
{
    if (diri.any()) {
        if (Time.appTime() >= time) {
            if (time < 0) {
                time = Time.appTime() + first_time;
            } else {
                time = Time.appTime() + repeat_time;
                diri_r += diri;
            }
        }
    } else {
        time = -FLT_MAX;
    }
}
static void UpdateDirRep(VecSB2 &diri_r, C VecSB2 &diri, Flt &time, Flt first_time, Flt repeat_time) {
    if (diri.any()) {
        if (Time.appTime() >= time) {
            diri_r += diri;
            time = Time.appTime() + ((time < 0) ? first_time : repeat_time); // if first press, then wait longer
        }
    } else {
        time = -FLT_MAX;
    }
}
static VecSB2 DirToDirI(C Vec2 &d) {
#if 0
   const Flt tan=0.414213568f; // Tan(PI_4/2) correct value
#else
    const Flt tan = 0.5f; // use higher value to minimize chance of moving diagonally by accident
#endif
    const Flt dead = 0.333f;
    Flt v = Abs(d).max();
    if (v > dead)
        return SignEpsB(d / v, tan);
    return 0;
}
#if JOYPAD_THREAD
#if WINDOWS_NEW
static inline Bool FlagOn(Windows::Gaming::Input::GamepadButtons flags, Windows::Gaming::Input::GamepadButtons f) { return (flags & f) != Windows::Gaming::Input::GamepadButtons::None; }
#endif
static inline void Test(Bool old, Bool cur, Byte button, Byte device) {
    if (old != cur)
        ThreadInputs.New().set(cur, INPUT_JOYPAD, button, device);
}
T2(TYPE, TYPE1)
static inline void Test(TYPE changed, TYPE cur, TYPE1 test, Byte button, Byte device) {
    if (FlagOn(changed, test))
        ThreadInputs.New().set(FlagOn(cur, test), INPUT_JOYPAD, button, device);
}
static inline void TestDPadX(Int old, Int cur, Byte device) // have to process in a special way to make sure we release first and push next
{
    if (old != cur) {
        if (old)
            ThreadInputs.New().set(false, INPUT_JOYPAD, (old > 0) ? JB_DPAD_RIGHT : JB_DPAD_LEFT, device); // release first
        if (cur)
            ThreadInputs.New().set(true, INPUT_JOYPAD, (cur > 0) ? JB_DPAD_RIGHT : JB_DPAD_LEFT, device); // push
    }
}
static inline void TestDPadY(Int old, Int cur, Byte device) // have to process in a special way to make sure we release first and push next
{
    if (old != cur) {
        if (old)
            ThreadInputs.New().set(false, INPUT_JOYPAD, (old > 0) ? JB_DPAD_UP : JB_DPAD_DOWN, device); // release first
        if (cur)
            ThreadInputs.New().set(true, INPUT_JOYPAD, (cur > 0) ? JB_DPAD_UP : JB_DPAD_DOWN, device); // push
    }
}
#if JP_GAMEPAD_INPUT
#if WINDOWS_OLD
static VecI2 SwitchToDirI(ABI::Windows::Gaming::Input::GameControllerSwitchPosition pos)
#else
static VecI2 SwitchToDirI(Windows::Gaming::Input::GameControllerSwitchPosition pos)
#endif
{
    switch (pos) {
#if WINDOWS_OLD
    default:
        return VecI2(0, 0); // ABI::Windows::Gaming::Input::GameControllerSwitchPosition_Center
    case ABI::Windows::Gaming::Input::GameControllerSwitchPosition_Up:
        return VecI2(0, 1);
    case ABI::Windows::Gaming::Input::GameControllerSwitchPosition_UpRight:
        return VecI2(1, 1);
    case ABI::Windows::Gaming::Input::GameControllerSwitchPosition_Right:
        return VecI2(1, 0);
    case ABI::Windows::Gaming::Input::GameControllerSwitchPosition_DownRight:
        return VecI2(1, -1);
    case ABI::Windows::Gaming::Input::GameControllerSwitchPosition_Down:
        return VecI2(0, -1);
    case ABI::Windows::Gaming::Input::GameControllerSwitchPosition_DownLeft:
        return VecI2(-1, -1);
    case ABI::Windows::Gaming::Input::GameControllerSwitchPosition_Left:
        return VecI2(-1, 0);
    case ABI::Windows::Gaming::Input::GameControllerSwitchPosition_UpLeft:
        return VecI2(-1, 1);
#else
    default:
        return VecI2(0, 0); // Windows::Gaming::Input::GameControllerSwitchPosition::Center
    case Windows::Gaming::Input::GameControllerSwitchPosition::Up:
        return VecI2(0, 1);
    case Windows::Gaming::Input::GameControllerSwitchPosition::UpRight:
        return VecI2(1, 1);
    case Windows::Gaming::Input::GameControllerSwitchPosition::Right:
        return VecI2(1, 0);
    case Windows::Gaming::Input::GameControllerSwitchPosition::DownRight:
        return VecI2(1, -1);
    case Windows::Gaming::Input::GameControllerSwitchPosition::Down:
        return VecI2(0, -1);
    case Windows::Gaming::Input::GameControllerSwitchPosition::DownLeft:
        return VecI2(-1, -1);
    case Windows::Gaming::Input::GameControllerSwitchPosition::Left:
        return VecI2(-1, 0);
    case Windows::Gaming::Input::GameControllerSwitchPosition::UpLeft:
        return VecI2(-1, 1);
#endif
    }
}
#endif
#if JP_DIRECT_INPUT
static VecI2 POVToDirI(DWORD pov) {
    switch (pov) {
    case UINT_MAX:
        return VecI2(0, 0);
    case 0:
        return VecI2(0, 1);
    case 4500:
        return VecI2(1, 1);
    case 9000:
        return VecI2(1, 0);
    case 13500:
        return VecI2(1, -1);
    case 18000:
        return VecI2(0, -1);
    case 22500:
        return VecI2(-1, -1);
    case 27000:
        return VecI2(-1, 0);
    case 31500:
        return VecI2(-1, 1);
    default:
        Vec2 dir;
        CosSin(dir.x, dir.y, PI_2 - DegToRad(pov / 100.0f));
        return Round(dir);
    }
}
#endif
void Joypad::getState() {
    auto &olds = _state[_state_index],
         &curs = _state[!_state_index]; // here indexes for 'cur' have to be swapped when compared to 'update' because here after success we flip '_state_index', so later when wanting to access 'cur' we need to do "_state[_state_index]"
#if JP_GAMEPAD_INPUT
    if (_gamepad) {
        auto &old = olds.gamepad,
             &cur = curs.gamepad;
#if WINDOWS_OLD
        if (OK(_gamepad->GetCurrentReading(&cur)))
#else
        cur = _gamepad->GetCurrentReading();
#endif
        {
            _state_index ^= 1;
            if (old.Buttons != cur.Buttons) {
                auto changed = old.Buttons ^ cur.Buttons;
#if WINDOWS_OLD
                // buttons
                Test(changed, cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_A, JB_A, _joypad_index);
                Test(changed, cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_B, JB_B, _joypad_index);
                Test(changed, cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_X, JB_X, _joypad_index);
                Test(changed, cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_Y, JB_Y, _joypad_index);
                Test(changed, cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_LeftShoulder, JB_L1, _joypad_index);
                Test(changed, cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_RightShoulder, JB_R1, _joypad_index);
                Test(changed, cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_LeftThumbstick, JB_L3, _joypad_index);
                Test(changed, cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_RightThumbstick, JB_R3, _joypad_index);
                Test(changed, cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_View, JB_BACK, _joypad_index);
                Test(changed, cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_Menu, JB_START, _joypad_index);
#if 0
            Test(changed, cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_Paddle1        , JB_PADDLE1, _joypad_index);
            Test(changed, cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_Paddle2        , JB_PADDLE2, _joypad_index);
            Test(changed, cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_Paddle3        , JB_PADDLE3, _joypad_index);
            Test(changed, cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_Paddle4        , JB_PADDLE4, _joypad_index);
#endif

                // dpad
                if (FlagOn(changed, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_DPadLeft | ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_DPadRight | ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_DPadDown | ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_DPadUp)) {
                    TestDPadX(FlagOn(old.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_DPadRight) - FlagOn(old.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_DPadLeft), FlagOn(cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_DPadRight) - FlagOn(cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_DPadLeft), _joypad_index);
                    TestDPadY(FlagOn(old.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_DPadUp) - FlagOn(old.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_DPadDown), FlagOn(cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_DPadUp) - FlagOn(cur.Buttons, ABI::Windows::Gaming::Input::GamepadButtons::GamepadButtons_DPadDown), _joypad_index);
                }
#else
                // buttons
                Test(changed, cur.Buttons, Windows::Gaming::Input::GamepadButtons::A, JB_A, _joypad_index);
                Test(changed, cur.Buttons, Windows::Gaming::Input::GamepadButtons::B, JB_B, _joypad_index);
                Test(changed, cur.Buttons, Windows::Gaming::Input::GamepadButtons::X, JB_X, _joypad_index);
                Test(changed, cur.Buttons, Windows::Gaming::Input::GamepadButtons::Y, JB_Y, _joypad_index);
                Test(changed, cur.Buttons, Windows::Gaming::Input::GamepadButtons::LeftShoulder, JB_L1, _joypad_index);
                Test(changed, cur.Buttons, Windows::Gaming::Input::GamepadButtons::RightShoulder, JB_R1, _joypad_index);
                Test(changed, cur.Buttons, Windows::Gaming::Input::GamepadButtons::LeftThumbstick, JB_L3, _joypad_index);
                Test(changed, cur.Buttons, Windows::Gaming::Input::GamepadButtons::RightThumbstick, JB_R3, _joypad_index);
                Test(changed, cur.Buttons, Windows::Gaming::Input::GamepadButtons::View, JB_BACK, _joypad_index);
                Test(changed, cur.Buttons, Windows::Gaming::Input::GamepadButtons::Menu, JB_START, _joypad_index);
#if 0
            Test(changed, cur.Buttons, Windows::Gaming::Input::GamepadButtons::Paddle1        , JB_PADDLE1, _joypad_index);
            Test(changed, cur.Buttons, Windows::Gaming::Input::GamepadButtons::Paddle2        , JB_PADDLE2, _joypad_index);
            Test(changed, cur.Buttons, Windows::Gaming::Input::GamepadButtons::Paddle3        , JB_PADDLE3, _joypad_index);
            Test(changed, cur.Buttons, Windows::Gaming::Input::GamepadButtons::Paddle4        , JB_PADDLE4, _joypad_index);
#endif

                // dpad
                if (FlagOn(changed, Windows::Gaming::Input::GamepadButtons::DPadLeft | Windows::Gaming::Input::GamepadButtons::DPadRight | Windows::Gaming::Input::GamepadButtons::DPadDown | Windows::Gaming::Input::GamepadButtons::DPadUp)) {
                    TestDPadX(FlagOn(old.Buttons, Windows::Gaming::Input::GamepadButtons::DPadRight) - FlagOn(old.Buttons, Windows::Gaming::Input::GamepadButtons::DPadLeft), FlagOn(cur.Buttons, Windows::Gaming::Input::GamepadButtons::DPadRight) - FlagOn(cur.Buttons, Windows::Gaming::Input::GamepadButtons::DPadLeft), _joypad_index);
                    TestDPadY(FlagOn(old.Buttons, Windows::Gaming::Input::GamepadButtons::DPadUp) - FlagOn(old.Buttons, Windows::Gaming::Input::GamepadButtons::DPadDown), FlagOn(cur.Buttons, Windows::Gaming::Input::GamepadButtons::DPadUp) - FlagOn(cur.Buttons, Windows::Gaming::Input::GamepadButtons::DPadDown), _joypad_index);
                }
#endif
            }

            // triggers
            const Dbl eps = 30.0 / 255; // matches XINPUT_GAMEPAD_TRIGGER_THRESHOLD
            if (old.LeftTrigger != cur.LeftTrigger)
                Test(old.LeftTrigger >= eps, cur.LeftTrigger >= eps, JB_L2, _joypad_index);
            if (old.RightTrigger != cur.RightTrigger)
                Test(old.RightTrigger >= eps, cur.RightTrigger >= eps, JB_R2, _joypad_index);
        }
    } else if (_raw_game_controller) {
#if WINDOWS_OLD
        auto &old = olds.raw_game_controller,
             &cur = curs.raw_game_controller;
        UINT64 timestamp;
        if (OK(_raw_game_controller->GetCurrentReading(_buttons, cur.button, _switches, cur.Switch, _axes, cur.axis, &timestamp))) {
            auto &cur_button = cur.button, &old_button = old.button;
            auto &cur_switch = cur.Switch, &old_switch = old.Switch;
            // auto &cur_axis  =cur.axis  , &old_axis  =old.axis  ;
#else
        auto &old = olds,
             &cur = curs;
        ULong timestamp = _raw_game_controller->GetCurrentReading(cur.button, cur.Switch, cur.axis);
        {
            auto cur_button = cur.button->Data, old_button = old.button->Data;
            auto cur_switch = cur.Switch->Data, old_switch = old.Switch->Data;
            // auto cur_axis  =cur.axis  ->Data, old_axis  =old.axis  ->Data;
#endif
            _state_index ^= 1;
            REP(_buttons) {
                auto cur = cur_button[i];
                if (old_button[i] != cur) {
                    Byte button = _remap[i];
                    if (InRange(button, _button))
                        ThreadInputs.New().set(cur, INPUT_JOYPAD, button, _joypad_index);
                }
            }
            if (_switches && old_switch[0] != cur_switch[0]) {
                VecI2 old_diri = SwitchToDirI(old_switch[0]),
                      cur_diri = SwitchToDirI(cur_switch[0]);
                TestDPadX(old_diri.x, cur_diri.x, _joypad_index);
                TestDPadY(old_diri.y, cur_diri.y, _joypad_index);
            }
        }
    }
#endif
#if JP_X_INPUT
    if (_xinput != 255) {
        auto &old = olds.xinput,
             &cur = curs.xinput;
        if (XInputGetState(_xinput, &cur) == ERROR_SUCCESS) {
            _state_index ^= 1;
            if (old.Gamepad.wButtons != cur.Gamepad.wButtons) {
                // buttons
                WORD changed = old.Gamepad.wButtons ^ cur.Gamepad.wButtons;
                Test(changed, cur.Gamepad.wButtons, XINPUT_GAMEPAD_A, JB_A, _joypad_index);
                Test(changed, cur.Gamepad.wButtons, XINPUT_GAMEPAD_B, JB_B, _joypad_index);
                Test(changed, cur.Gamepad.wButtons, XINPUT_GAMEPAD_X, JB_X, _joypad_index);
                Test(changed, cur.Gamepad.wButtons, XINPUT_GAMEPAD_Y, JB_Y, _joypad_index);
                Test(changed, cur.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER, JB_L1, _joypad_index);
                Test(changed, cur.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER, JB_R1, _joypad_index);
                Test(changed, cur.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB, JB_L3, _joypad_index);
                Test(changed, cur.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB, JB_R3, _joypad_index);
                Test(changed, cur.Gamepad.wButtons, XINPUT_GAMEPAD_BACK, JB_BACK, _joypad_index);
                Test(changed, cur.Gamepad.wButtons, XINPUT_GAMEPAD_START, JB_START, _joypad_index);

                // dpad
                if (FlagOn(changed, XINPUT_GAMEPAD_DPAD_LEFT | XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_UP)) {
                    TestDPadX(FlagOn(old.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT) - FlagOn(old.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT), FlagOn(cur.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT) - FlagOn(cur.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT), _joypad_index);
                    TestDPadY(FlagOn(old.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP) - FlagOn(old.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN), FlagOn(cur.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP) - FlagOn(cur.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN), _joypad_index);
                }
            }
            // triggers
            if (old.Gamepad.bLeftTrigger != cur.Gamepad.bLeftTrigger)
                Test(old.Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD, cur.Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD, JB_L2, _joypad_index);
            if (old.Gamepad.bRightTrigger != cur.Gamepad.bRightTrigger)
                Test(old.Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD, cur.Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD, JB_R2, _joypad_index);
        }
    }
#endif
#if JP_DIRECT_INPUT
    if (_dinput) {
        auto &old = olds.dinput,
             &cur = curs.dinput;
        if (OK(_dinput->Poll()) && OK(_dinput->GetDeviceState(SIZE(cur), &cur))) {
            _state_index ^= 1;

            // buttons
            REP(Min(Elms(cur.rgbButtons), Elms(_remap))) {
                auto cur_b = cur.rgbButtons[i];
                if (old.rgbButtons[i] != cur_b) {
                    Byte button = _remap[i];
                    if (InRange(button, _button))
                        ThreadInputs.New().set(cur_b != 0, INPUT_JOYPAD, button, _joypad_index); // here values can be 0x80
                }
            }

            // dpad
            if (old.rgdwPOV[0] != cur.rgdwPOV[0]) {
                VecI2 old_diri = POVToDirI(old.rgdwPOV[0]),
                      cur_diri = POVToDirI(cur.rgdwPOV[0]);
                TestDPadX(old_diri.x, cur_diri.x, _joypad_index);
                TestDPadY(old_diri.y, cur_diri.y, _joypad_index);
            }
        }
    }
#endif
}
#endif
#if IOS
void Joypad::addPad(GCControllerElement *element) {
    if (element)
        _elms.New().set(Elm::PAD, 0, element);
}
void Joypad::addButton(GCControllerElement *element, JOYPAD_BUTTON button) {
    if (element)
        _elms.New().set(Elm::BUTTON, button, element);
}
void Joypad::addTrigger(GCControllerElement *element, Byte index) {
    if (element)
        _elms.New().set(Elm::TRIGGER, index, element);
}
void Joypad::addAxis(GCControllerElement *element, Byte index) {
    if (element)
        _elms.New().set(Elm::AXIS, index, element);
}

static Int Compare(C Joypad::Elm &a, GCControllerElement *C &b) { return ComparePtr(a.element, b); }

void Joypad::changed(GCControllerElement *element) {
    if (C Elm *elm = _elms.binaryFind(element, Compare))
        switch (elm->type) {
        case Elm::PAD: {
            GCControllerDirectionPad *v = (GCControllerDirectionPad *)element;
            setDiri(v.right.pressed - v.left.pressed, v.up.pressed - v.down.pressed);
            dir = diri;
            if (diri.x && diri.y)
                dir /= SQRT2;
        } break; // dir.clipLength(1)
        case Elm::BUTTON: {
            GCControllerButtonInput *v = (GCControllerButtonInput *)element;
            if (v.pressed)
                push(elm->index);
            else
                release(elm->index);
        } break;
        case Elm::TRIGGER: {
            GCControllerButtonInput *v = (GCControllerButtonInput *)element;
            setTrigger(elm->index, v.value);
        } break;
        case Elm::AXIS: {
            GCControllerDirectionPad *v = (GCControllerDirectionPad *)element;
            dir_a[elm->index].set(v.xAxis.value, v.yAxis.value);
        } break;
        }
}
#endif
void Joypad::update() {
#if JOYPAD_THREAD
    auto &cur = _state[_state_index];
#if JP_GAMEPAD_INPUT
    if (_gamepad) {
        auto &state = cur.gamepad;

        // analog pad
        dir_a[0].set(state.LeftThumbstickX, state.LeftThumbstickY);
        dir_a[1].set(state.RightThumbstickX, state.RightThumbstickY);

        // triggers
        trigger[0] = state.LeftTrigger;
        trigger[1] = state.RightTrigger;
    } else if (_raw_game_controller) {
#if WINDOWS_OLD
        // auto &Switch=cur.raw_game_controller.Switch;
        auto &axis = cur.raw_game_controller.axis;
#else
        // auto Switch=cur.Switch->Data;
        auto axis = cur.axis->Data;
#endif
        if (_axes >= 2)
            dir_a[0].set(axis[0] * 2 - 1, axis[1] * -2 + 1);
        if (_axis_stick_r_x != 0xFF)
            dir_a[1].set(axis[_axis_stick_r_x] * 2 - 1, axis[_axis_stick_r_y] * -2 + 1); // _axes>=4
        if (_axis_trigger_l != 0xFF) {
            trigger[0] = axis[_axis_trigger_l];
            trigger[1] = axis[_axis_trigger_r];
        } // _axes>=6
    }
#endif
#if JP_X_INPUT
    if (_xinput != 255) {
        auto &state = cur.xinput;

        // analog pad
        dir_a[0].x = state.Gamepad.sThumbLX / 32768.0f;
        dir_a[0].y = state.Gamepad.sThumbLY / 32768.0f;
        dir_a[1].x = state.Gamepad.sThumbRX / 32768.0f;
        dir_a[1].y = state.Gamepad.sThumbRY / 32768.0f;

        // triggers
        trigger[0] = state.Gamepad.bLeftTrigger / 255.0f;
        trigger[1] = state.Gamepad.bRightTrigger / 255.0f;
    }
#endif
#if JP_DIRECT_INPUT
    if (_dinput) {
        auto &state = cur.dinput;

        // analog pad
        dir_a[0].x = (state.lX - 32768) / 32768.0f;
        dir_a[0].y = -(state.lY - 32768) / 32768.0f;
        if (_axis_stick_r_x && _axis_stick_r_y) {
            ASSERT(SIZE(state.lZ) == SIZE(Int));
            dir_a[1].x = (*(Int *)(((Byte *)&state) + _axis_stick_r_x) - 32768) / 32768.0f;
            dir_a[1].y = -(*(Int *)(((Byte *)&state) + _axis_stick_r_y) - 32768) / 32768.0f;
        }

        // triggers
        // trigger[0]=(state.rglSlider[0]-32768)/32768.0f;
        // trigger[1]=(state.rglSlider[1]-32768)/32768.0f;
    }
#endif
#endif
    UpdateDirRep1(diri_r, diri, _dir_t, FirstRepeatPressTime, RepeatPressTime); // skip first add because we're already processing that in 'setDiri'
    UpdateDirRep(diri_ar[0], DirToDirI(dir_a[0]), _dir_at[0], FirstRepeatPressTime, AnalogRepeatPressTime);
    UpdateDirRep(diri_ar[1], DirToDirI(dir_a[1]), _dir_at[1], FirstRepeatPressTime, AnalogRepeatPressTime);
}
void Joypad::setDiri(Int x, Int y) {
    if (diri.x != x) {
        if (diri.x /*&& _joypad_index!=255*/)
            Inputs.New().set(false, INPUT_JOYPAD, (diri.x > 0) ? JB_DPAD_RIGHT : JB_DPAD_LEFT, _joypad_index); // release first
        if (diri.x = x) {                                                                                      /*if(_joypad_index!=255)*/
            Inputs.New().set(true, INPUT_JOYPAD, (diri.x > 0) ? JB_DPAD_RIGHT : JB_DPAD_LEFT, _joypad_index);
            diri_r.x += x;
        } // push
    }
    if (diri.y != y) {
        if (diri.y /*&& _joypad_index!=255*/)
            Inputs.New().set(false, INPUT_JOYPAD, (diri.y > 0) ? JB_DPAD_UP : JB_DPAD_DOWN, _joypad_index); // release first
        if (diri.y = y) {                                                                                   /*if(_joypad_index!=255)*/
            Inputs.New().set(true, INPUT_JOYPAD, (diri.y > 0) ? JB_DPAD_UP : JB_DPAD_DOWN, _joypad_index);
            diri_r.y += y;
        } // push
    }
}
void Joypad::setTrigger(Int index, Flt value) {
    Flt &old = trigger[index];
    if (old != value) {
        const Flt eps = 30.0f / 255; // matches XINPUT_GAMEPAD_TRIGGER_THRESHOLD
        Bool old_b = (old >= eps), cur_b = (value >= eps);
        if (old_b != cur_b) {
            auto button = (index ? JB_R2 : JB_L2);
            if (cur_b)
                push(button);
            else
                release(button);
        }
        old = value;
    }
}
void Joypad::push(Byte b) {
    if (InRange(b, _button) && !(_button[b] & BS_ON)) {
        _button[b] |= BS_PUSHED | BS_ON;
        if (Time.appTime() - _last_t[b] <= DoubleClickTime + Time.ad()) {
            _button[b] |= BS_DOUBLE;
            _last_t[b] = -FLT_MAX;
        } else {
            _last_t[b] = Time.appTime();
        }
        /*if(_joypad_index!=255)*/ Inputs.New().set(true, INPUT_JOYPAD, b, _joypad_index);
    }
}
void Joypad::release(Byte b) {
    if (InRange(b, _button) && (_button[b] & BS_ON)) {
        _button[b] &= ~BS_ON;
        _button[b] |= BS_RELEASED;
        /*if(_joypad_index!=255)*/ Inputs.New().set(false, INPUT_JOYPAD, b, _joypad_index);
    }
}
void Joypad::eat(Int b) {
    if (InRange(b, _button))
        FlagDisable(_button[b], BS_NOT_ON);
}
/******************************************************************************/
void Joypad::acquire(Bool on) {
#if JP_DIRECT_INPUT
    if (_dinput) {
        if (on)
            _dinput->Acquire();
        else
            _dinput->Unacquire();
    }
#endif
    if (!on               // unacquire
        || JOYPAD_THREAD) // zero even when wanting to acquire if processing joypad threads, in case some state changes got updated on the joypad thread
    {
        // this will generate release 'Inputs' events
        // zero is also needed for Nintendo Switch in case joypad changed 'mini' because not all members get updated for it
        setDiri(0, 0);
        REPA(_button)
        release(i);
        zero();
    }
}
#if !SWITCH
void Joypad::sensors(Bool calculate) {}
#endif
/******************************************************************************/
// JOYPADS
/******************************************************************************/
void ApplyDeadZone(Vec2 &v, Flt dead_zone) {
    Flt len = v.length();
    if (len <= dead_zone)
        v.zero();
    else // zero
        if (len >= 1)
            v /= len;
        else                                     // normalize
            v *= LerpR(dead_zone, 1, len) / len; // scale from dead_zone..1 -> 0..1
}
/******************************************************************************/
Bool JoypadSensors() { return CalculateJoypadSensors; }
void JoypadSensors(Bool calculate) {
    if (CalculateJoypadSensors != calculate) {
        CalculateJoypadSensors ^= 1;
        REPAO(Joypads).sensors(CalculateJoypadSensors);
    }
}
/******************************************************************************/
#if !SWITCH
void ConfigureJoypads(Int min_players, Int max_players, C CMemPtr<Str> &player_names, C CMemPtr<Color> &player_colors) {}
#endif
/******************************************************************************/
static void RemapSwap(Memc<Input> &inputs, Int a, Int b) {
    REPA(inputs) {
        Input &input = inputs[i];
        if (input.type == INPUT_JOYPAD) {
            if (input.device == a)
                input.device = b;
            else if (input.device == b)
                input.device = a;
        }
    }
}
static void RemapMove(Memc<Input> &inputs, Int elm, Int new_index) {
    REPA(inputs) {
        Input &input = inputs[i];
        if (input.type == INPUT_JOYPAD) {
            if (input.device == elm)
                input.device = new_index;
            else if (input.device < elm && input.device >= new_index)
                input.device++;
            else // 'elm' is moved left , so indexes between 'new_index' and 'elm' should move right
                if (input.device > elm && input.device <= new_index)
                    input.device--; // 'elm' is moved right, so indexes between 'new_index' and 'elm' should move left
        }
    }
}
void JoypadsClass::swapOrder(Int i, Int j) {
    if (InRange(i, T) && InRange(j, T) && i != j) {
#if JOYPAD_THREAD
        SyncLocker lock(JoypadLock);
#endif
        RemapSwap(Inputs, i, j);
#if JOYPAD_THREAD
        RemapSwap(ThreadInputs, i, j);
#endif
        _data.swapOrder(i, j);
        REPAO(_data)._joypad_index = i;
        if (auto func = App.joypad_changed)
            func();
    }
}
void JoypadsClass::moveElm(Int elm, Int new_index) {
    if (InRange(elm, T) && InRange(new_index, T) && elm != new_index) {
#if JOYPAD_THREAD
        SyncLocker lock(JoypadLock);
#endif
        RemapMove(Inputs, elm, new_index);
#if JOYPAD_THREAD
        RemapMove(ThreadInputs, elm, new_index);
#endif
        _data.moveElm(elm, new_index);
        REPAO(_data)._joypad_index = i;
        if (auto func = App.joypad_changed)
            func();
    }
}
/******************************************************************************/
void JoypadsClass::clear() {
#if JOYPAD_THREAD
    if (_data.elms()) {
        SyncLocker lock(JoypadLock);
#else
    {
#endif
        _data.clear();
        if (auto func = App.joypad_changed)
            func();
    }
}
void JoypadsClass::remove(Int i) {
#if JOYPAD_THREAD
    if (InRange(i, T)) {
        SyncLocker lock(JoypadLock);
#else
    {
#endif
        _data.remove(i, true);
        if (auto func = App.joypad_changed)
            func();
    }
}
void JoypadsClass::remove(Joypad *joypad) {
    if (joypad)
        remove(joypad->_joypad_index); // _data.index(joypad)
}
static Int Compare(C Joypad &a, C UInt &b) { return Compare(a.id(), b); }
Joypad *JoypadsClass::find(UInt id) {
    REPA(T) {
        Joypad &jp = T[i];
        if (jp.id() == id)
            return &jp;
    }
    return null;
}
Joypad &GetJoypad(UInt id, Bool &added) {
#if JOYPAD_THREAD
    DEBUG_ASSERT(JoypadLock.owned(), "'GetJoypad' must be called only under lock");
#endif
    added = false;
    Joypad *joypad = Joypads.find(id);
    if (!joypad) {
        joypad = &Joypads._data.New();
        joypad->_id = id;
        added = true;
    }
    joypad->_connected = true;
    return *joypad;
}
UInt NewJoypadID(UInt id) {
    for (;;) {
        if (!Joypads.find(id))
            return id; // if no joypad uses this ID, then we can use it
        id++;          // increase
    }
}
/******************************************************************************/
#if JP_DIRECT_INPUT
static Bool IsGamepadInput(U16 vendor_id, U16 product_id) {
#if JP_GAMEPAD_INPUT
    REPA(Joypads) {
        Joypad &jp = Joypads[i];
        if (jp._vendor_id == vendor_id && jp._product_id == product_id && (jp._gamepad || jp._raw_game_controller))
            return true;
    }
#endif
    return false;
}
static Bool IsXInputDevice(C GUID &pGuidProductFromDirectInput) // !! Warning: this might trigger calling 'WindowMsg' !!
{
    Bool xinput = false;

    // Create WMI
    IWbemLocator *pIWbemLocator = null;
    CoCreateInstance(__uuidof(WbemLocator), null, CLSCTX_INPROC_SERVER, __uuidof(IWbemLocator), (Ptr *)&pIWbemLocator);
    if (pIWbemLocator) {
        // Using the locator, connect to WMI in the given namespace
        if (BSTR Namespace = SysAllocString(L"root\\cimv2")) {
            IWbemServices *pIWbemServices = null;
            pIWbemLocator->ConnectServer(Namespace, null, null, null, 0, null, null, &pIWbemServices);
            if (pIWbemServices) {
                if (BSTR Win32_PNPEntity = SysAllocString(L"Win32_PNPEntity")) {
                    CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, null, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, null, EOAC_NONE); // Switch security level to IMPERSONATE
                    IEnumWbemClassObject *pEnumDevices = null;
                    pIWbemServices->CreateInstanceEnum(Win32_PNPEntity, 0, null, &pEnumDevices);
                    if (pEnumDevices) {
                        if (BSTR DeviceID = SysAllocString(L"DeviceID")) {
                            IWbemClassObject *pDevices[16];
                        again:
                            DWORD returned = 0;
                            if (OK(pEnumDevices->Next(10000, Elms(pDevices), pDevices, &returned)) && returned) {
                                FREP(returned) // check each device
                                {
                                    if (!xinput) {
                                        VARIANT var;
                                        if (OK(pDevices[i]->Get(DeviceID, 0, &var, null, null))) {
                                            if (var.vt == VT_BSTR && var.bstrVal != null && wcsstr(var.bstrVal, L"IG_")) // Check if the device ID contains "IG_". If it does, then it's an XInput device
                                            {
                                                DWORD vendor_id = 0, product_id = 0; // If it does, then get the VID/PID from var.bstrVal
#pragma warning(push)
#pragma warning(disable : 4996)
                                                WCHAR *strVid = wcsstr(var.bstrVal, L"VID_");
                                                if (strVid && swscanf(strVid, L"VID_%4X", &vendor_id) != 1)
                                                    vendor_id = 0;
                                                WCHAR *strPid = wcsstr(var.bstrVal, L"PID_");
                                                if (strPid && swscanf(strPid, L"PID_%4X", &product_id) != 1)
                                                    product_id = 0;
#pragma warning(pop)

                                                if (MAKELONG(vendor_id, product_id) == pGuidProductFromDirectInput.Data1) // Compare the VID/PID to the DInput device
                                                    xinput = true;
                                            }
                                            VariantClear(&var);
                                        }
                                    }
                                    RELEASE(pDevices[i]);
                                }
                                if (!xinput)
                                    goto again;
                            }
                            SysFreeString(DeviceID);
                        }
                        pEnumDevices->Release();
                    }
                    SysFreeString(Win32_PNPEntity);
                }
                pIWbemServices->Release();
            }
            SysFreeString(Namespace);
        }
        pIWbemLocator->Release();
    }
    return xinput;
}
static BOOL CALLBACK EnumAxes(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *user) {
    Joypad &joypad = *(Joypad *)user;

    // Logitech RumblePad 2                     uses: (x0=lX, y0=lY, x1=lZ , y1=lRz)
    // Logitech F310        in DirectInput mode uses: (x0=lX, y0=lY, x1=lZ , y1=lRz)
    // Logitech F310        in XInput      mode uses: (x0=lX, y0=lY, x1=lRx, y1=lRy)
    Int offset = 0;
    if (pdidoi->guidType == GUID_ZAxis)
        offset = OFFSET(DIJOYSTATE, lZ);
    else if (pdidoi->guidType == GUID_RxAxis)
        offset = OFFSET(DIJOYSTATE, lRx);
    else if (pdidoi->guidType == GUID_RyAxis)
        offset = OFFSET(DIJOYSTATE, lRy);
    else if (pdidoi->guidType == GUID_RzAxis)
        offset = OFFSET(DIJOYSTATE, lRz);

    if (offset) {
        if (!joypad._axis_stick_r_x || offset < joypad._axis_stick_r_x) // X axis is assumed to be specified before Y axis
        {
            joypad._axis_stick_r_y = joypad._axis_stick_r_x;
            joypad._axis_stick_r_x = offset;
        } else if (!joypad._axis_stick_r_y || offset < joypad._axis_stick_r_y) // Y axis is assumed to be specified before other axes
        {
            joypad._axis_stick_r_y = offset;
        }
    }

    return DIENUM_CONTINUE;
}
static BOOL CALLBACK EnumJoypads(const DIDEVICEINSTANCE *DIDevInst, void *) {
    U16 vendor_id = LOWORD(DIDevInst->guidProduct.Data1);
    U16 product_id = HIWORD(DIDevInst->guidProduct.Data1);
#if JP_X_INPUT
    if (!IsXInputDevice(DIDevInst->guidProduct)) // if we're using XInput then don't list this device here, it will be processed by XInput
#endif
#if JP_GAMEPAD_INPUT
        if (!IsGamepadInput(vendor_id, product_id)) // if we're using GamepadInput then don't list this device here, it will be processed by GamepadInput
#endif
        {
            UInt id = 0;
            ASSERT(SIZE(DIDevInst->guidInstance) == SIZE(UID));
            C UID &uid = (UID &)DIDevInst->guidInstance;
            REPA(uid.i)
            id ^= uid.i[i];
            // here no need for lock, because this functions is called under lock already
            Bool added;
            Joypad &joypad = GetJoypad(id, added);
            if (added) {
                IDirectInputDevice8 *did = null;
                if (OK(InputDevices.DI->CreateDevice(DIDevInst->guidInstance, &did, null)))
                    if (OK(did->SetDataFormat(&c_dfDIJoystick)))
                        if (OK(did->SetCooperativeLevel(App.window(), DISCL_EXCLUSIVE | DISCL_FOREGROUND))) {
                            Swap(joypad._dinput, did);
                            joypad._name = DIDevInst->tszProductName;

                            REPAO(joypad._state).dinput.rgdwPOV[0] = UINT_MAX; // set initial DPAD to centered

                            // disable auto centering ?
                            DIPROPDWORD dipdw;
                            Zero(dipdw);
                            dipdw.diph.dwSize = SIZE(dipdw);
                            dipdw.diph.dwHeaderSize = SIZE(DIPROPHEADER);
                            dipdw.diph.dwHow = DIPH_DEVICE;
                            joypad._dinput->SetProperty(DIPROP_AUTOCENTER, &dipdw.diph);

                            joypad._dinput->EnumObjects(EnumAxes, &joypad, DIDFT_AXIS);

                            joypad.setInfo(vendor_id, product_id);

                            if (App.active())
                                joypad.acquire(true);
                        }
                RELEASE(did);

                if (joypad._dinput) {
                    if (auto func = App.joypad_changed)
                        func();
                } // call at the end
                else
                    Joypads.remove(joypad._joypad_index); // if failed to create it then remove it, this will already call 'App.joypad_changed'
            }
        }
    return DIENUM_CONTINUE;
}
#endif
/******************************************************************************/
void ListJoypads() {
#if JP_X_INPUT || JP_DIRECT_INPUT
#if JOYPAD_THREAD
    SyncLocker lock(JoypadLock);
#endif
    REPA(Joypads) // assume that all are disconnected
    {
        Joypad &jp = Joypads[i];
#if JP_X_INPUT
        if (jp._xinput != 255)
            jp._connected = false;
#endif
#if JP_DIRECT_INPUT
        if (jp._dinput)
            jp._connected = false;
#endif
    }

#if JP_GAMEPAD_INPUT && WINDOWS_OLD && 0
    if (GamepadStatics)
        this is handled in GamepadAdded / GamepadRemoved {
            Microsoft::WRL::ComPtr<ABI::Windows::Foundation::Collections::IVectorView<ABI::Windows::Gaming::Input::Gamepad *>> gamepads;
            GamepadStatics->get_Gamepads(&gamepads);
            if (gamepads) {
                unsigned gamepad_count = 0;
                gamepads->get_Size(&gamepad_count);
                for (unsigned i = 0; i < gamepad_count; i++) {
                    Microsoft::WRL::ComPtr<ABI::Windows::Gaming::Input::IGamepad> gamepad;
                    gamepads->GetAt(i, &gamepad);
                    if (gamepad) {
                    }
                }
            }
        }
#endif
#if JP_X_INPUT
    ASSERT(XUSER_MAX_COUNT == 4);
    FREP(XUSER_MAX_COUNT) // XInput supports only 4 gamepads (process in order)
    {
        XINPUT_STATE state;
        if (XInputGetState(i, &state) == ERROR_SUCCESS) // if returned valid input
        {
            Bool added;
            Joypad &joypad = GetJoypad(i, added);
            if (added) // index is used for the ID for XInput gamepads
            {
                joypad._xinput = i;
                joypad._name = S + "X Gamepad #" + (i + 1);

                if (auto func = App.joypad_changed)
                    func(); // call at the end
            }
        }
    }
#endif
#if JP_DIRECT_INPUT
    if (InputDevices.DI)
        InputDevices.DI->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoypads, null, DIEDFL_ATTACHEDONLY /*|DIEDFL_FORCEFEEDBACK*/); // this would enumerate only devices with ForceFeedback
#endif

    REPA(Joypads)
    if (!Joypads[i]._connected)
        Joypads.remove(i); // remove disconnected joypads
#elif MAC
    if (HidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone)) {
        NSMutableDictionary *criteria[] =
            {
                JoypadCriteria(kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick),
                JoypadCriteria(kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad),
                JoypadCriteria(kHIDPage_GenericDesktop, kHIDUsage_GD_MultiAxisController),
            };
        NSArray *criteria_array = [NSArray arrayWithObjects:criteria[0], criteria[1], criteria[2], __null];
        IOHIDManagerSetDeviceMatchingMultiple(HidManager, (CFArrayRef)criteria_array);
        REPA(criteria)
        [criteria[i] release];

        IOHIDManagerScheduleWithRunLoop(HidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        IOReturn hid_open = IOHIDManagerOpen(HidManager, kIOHIDOptionsTypeNone);

        IOHIDManagerRegisterDeviceMatchingCallback(HidManager, JoypadAdded, null);
        IOHIDManagerRegisterDeviceRemovalCallback(HidManager, JoypadRemoved, null);
        IOHIDManagerRegisterInputValueCallback(HidManager, JoypadAction, null);
    }
#endif
}
/******************************************************************************/
inline static void Set(JP_NAME_TYPE type, Byte button, CChar8 *name, CChar *symbol) {
    JoypadButtonName[type][button] = name;
    JoypadButtonSymbol[type][button] = symbol;
}
void InitJoypads() {
    if (LogInit)
        LogN("InitJoypads");

    ASSERT(JB_TOTAL <= ELMS(JoypadButtonName[0]) && JB_DPAD_LEFT < ELMS(JoypadButtonName[0]) && JB_DPAD_RIGHT < ELMS(JoypadButtonName[0]) && JB_DPAD_DOWN < ELMS(JoypadButtonName[0]) && JB_DPAD_UP < ELMS(JoypadButtonName[0]));
    // set this first so other codes can overwrite it
    REPA(JoypadButtonName) {
        auto &name = JoypadButtonName[i];
        auto &symbol = JoypadButtonSymbol[i];
        name[0] = "1";
        symbol[0] = u"1";
        name[1] = "2";
        symbol[1] = u"2";
        name[2] = "3";
        symbol[2] = u"3";
        name[3] = "4";
        symbol[3] = u"4";
        name[4] = "5";
        symbol[4] = u"5";
        name[5] = "6";
        symbol[5] = u"6";
        name[6] = "7";
        symbol[6] = u"7";
        name[7] = "8";
        symbol[7] = u"8";
        name[8] = "9";
        symbol[8] = u"9";
        name[9] = "10";
        symbol[9] = u"10";
        name[10] = "11";
        symbol[10] = u"11";
        name[11] = "12";
        symbol[11] = u"12";
        name[12] = "13";
        symbol[12] = u"13";
        name[13] = "14";
        symbol[13] = u"14";
        name[14] = "15";
        symbol[14] = u"15";
        name[15] = "16";
        symbol[15] = u"16";
        name[16] = "17";
        symbol[16] = u"17";
        name[17] = "18";
        symbol[17] = u"18";
        name[18] = "19";
        symbol[18] = u"19";
        name[19] = "20";
        symbol[19] = u"20";
        name[20] = "21";
        symbol[20] = u"21";
        name[21] = "22";
        symbol[21] = u"22";
        name[22] = "23";
        symbol[22] = u"23";
        name[23] = "24";
        symbol[23] = u"24";
        name[24] = "25";
        symbol[24] = u"25";
        name[25] = "26";
        symbol[25] = u"26";
        name[26] = "27";
        symbol[26] = u"27";
        name[27] = "28";
        symbol[27] = u"28";
        name[28] = "29";
        symbol[28] = u"29";
        name[29] = "30";
        symbol[29] = u"30";
        name[30] = "31";
        symbol[30] = u"31";
        name[31] = "32";
        symbol[31] = u"32";
        ASSERT(31 < ELMS(JoypadButtonName[0]));

#if 0
      name[JB_PADDLE1]="Paddle1"; symbol[JB_PADDLE1]=u"Paddle1";
      name[JB_PADDLE2]="Paddle2"; symbol[JB_PADDLE2]=u"Paddle2";
      name[JB_PADDLE3]="Paddle3"; symbol[JB_PADDLE3]=u"Paddle3";
      name[JB_PADDLE4]="Paddle4"; symbol[JB_PADDLE4]=u"Paddle4";
#endif

        name[JB_DPAD_LEFT] = "Left";
        symbol[JB_DPAD_LEFT] = u"⯇";
        name[JB_DPAD_RIGHT] = "Right";
        symbol[JB_DPAD_RIGHT] = u"⯈";
        name[JB_DPAD_DOWN] = "Down";
        symbol[JB_DPAD_DOWN] = u"⯆";
        name[JB_DPAD_UP] = "Up";
        symbol[JB_DPAD_UP] = u"⯅";

        name[JB_LSL] = "Left SL";
        symbol[JB_LSL] = u"LeftSL";
        name[JB_LSR] = "Left SR";
        symbol[JB_LSR] = u"LeftSR";
        name[JB_RSL] = "Right SL";
        symbol[JB_RSL] = u"RightSL";
        name[JB_RSR] = "Right SR";
        symbol[JB_RSR] = u"RightSR";

        name[JB_HOME] = "Home";
        symbol[JB_HOME] = u"JB_HOME";
    }

    // set platform specific
    Set(JP_NAME, JB_L1, "Left Bumper", u"LB");
    Set(JP_NAME, JB_R1, "Right Bumper", u"RB");
    Set(JP_NAME_NINTENDO, JB_L1, "L", u"L");
    Set(JP_NAME_NINTENDO, JB_R1, "R", u"R");
    Set(JP_NAME_SONY, JB_L1, "L1", u"L1");
    Set(JP_NAME_SONY, JB_R1, "R1", u"R1");

    Set(JP_NAME, JB_L2, "Left Trigger", u"LT");
    Set(JP_NAME, JB_R2, "Right Trigger", u"RT");
    Set(JP_NAME_NINTENDO, JB_L2, "ZL", u"ZL");
    Set(JP_NAME_NINTENDO, JB_R2, "ZR", u"ZR");
    Set(JP_NAME_SONY, JB_L2, "L2", u"L2");
    Set(JP_NAME_SONY, JB_R2, "R2", u"R2");

    Set(JP_NAME, JB_L3, "Left Stick", u"LS");
    Set(JP_NAME, JB_R3, "Right Stick", u"RS");
    Set(JP_NAME_NINTENDO, JB_L3, "Left Stick", u"LS");
    Set(JP_NAME_NINTENDO, JB_R3, "Right Stick", u"RS");
    Set(JP_NAME_SONY, JB_L3, "Left Stick", u"L3");
    Set(JP_NAME_SONY, JB_R3, "Right Stick", u"R3");

    Set(JP_NAME, JB_BACK, "Back", u"Back");
    Set(JP_NAME, JB_START, "Start", u"Start");
    Set(JP_NAME_NINTENDO, JB_MINUS, "Minus", u"-");
    Set(JP_NAME_NINTENDO, JB_PLUS, "Plus", u"+");
    Set(JP_NAME_SONY, JB_SELECT, "Select", u"Select");
    Set(JP_NAME_SONY, JB_START, "Start", u"Start");

    Set(JP_NAME, JB_A, "A", u"A");
    Set(JP_NAME, JB_B, "B", u"B");
    Set(JP_NAME, JB_X, "X", u"X");
    Set(JP_NAME, JB_Y, "Y", u"Y");

    if (JoypadLayoutName) {
        Set(JP_NAME_NINTENDO, JB_A, "A", u"A");
        Set(JP_NAME_NINTENDO, JB_B, "B", u"B");
        Set(JP_NAME_NINTENDO, JB_X, "X", u"X");
        Set(JP_NAME_NINTENDO, JB_Y, "Y", u"Y");
    } else {
        Set(JP_NAME_NINTENDO, JB_R, "A", u"A");
        Set(JP_NAME_NINTENDO, JB_D, "B", u"B");
        Set(JP_NAME_NINTENDO, JB_U, "X", u"X");
        Set(JP_NAME_NINTENDO, JB_L, "Y", u"Y");
    }

    Set(JP_NAME_SONY, JB_L, "Square", u"□");
    Set(JP_NAME_SONY, JB_R, "Circle", u"○");
    Set(JP_NAME_SONY, JB_D, "Cross", u"✕");
    Set(JP_NAME_SONY, JB_U, "Triangle", u"△");

#if JP_GAMEPAD_INPUT && WINDOWS_OLD
    // !! SET ALL BEFORE ADDING CALLBACKS !!
    RoGetActivationFactory(Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_Gaming_Input_RawGameController).Get(), __uuidof(ABI::Windows::Gaming::Input::IRawGameControllerStatics), &RawGameControllerStatics);
    RoGetActivationFactory(Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_Gaming_Input_Gamepad).Get(), __uuidof(ABI::Windows::Gaming::Input::IGamepadStatics), &GamepadStatics);
    RoGetActivationFactory(Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_Gaming_Input_Gamepad).Get(), __uuidof(ABI::Windows::Gaming::Input::IGamepadStatics2), &GamepadStatics2);
    // !! ADD CALLBACKS AFTER !!
    if (GamepadStatics) {
        GamepadStatics->add_GamepadAdded(&GamepadAdded, &GamepadAddedToken);
        GamepadStatics->add_GamepadRemoved(&GamepadRemoved, &GamepadRemovedToken);
    }
    if (RawGameControllerStatics) {
        RawGameControllerStatics->add_RawGameControllerAdded(&RawGameControllerAdded, &RawGameControllerAddedToken);
        RawGameControllerStatics->add_RawGameControllerRemoved(&RawGameControllerRemoved, &RawGameControllerRemovedToken);
    }
#endif

    ListJoypads();

#if JP_RAW_INPUT
    RAWINPUTDEVICE rid[2];

    rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[0].usUsage = HID_USAGE_GENERIC_GAMEPAD;
    rid[0].dwFlags = RIDEV_INPUTSINK;
    rid[0].hwndTarget = App.window();
    rid[1] = rid[0];
    rid[1].usUsage = HID_USAGE_GENERIC_JOYSTICK;

    RegisterRawInputDevices(rid, Elms(rid), SIZE(RAWINPUTDEVICE));
#endif
}
/******************************************************************************/
void ShutJoypads() {
#if JP_RAW_INPUT
    RAWINPUTDEVICE rid[2];

    rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[0].usUsage = HID_USAGE_GENERIC_GAMEPAD;
    rid[0].dwFlags = RIDEV_REMOVE;
    rid[0].hwndTarget = App.window();
    rid[1] = rid[0];
    rid[1].usUsage = HID_USAGE_GENERIC_JOYSTICK;

    RegisterRawInputDevices(rid, Elms(rid), SIZE(RAWINPUTDEVICE));
#endif

#if JP_GAMEPAD_INPUT && WINDOWS_OLD
    // first remove callbacks
    if (GamepadStatics) {
        GamepadStatics->remove_GamepadAdded(GamepadAddedToken);
        GamepadStatics->remove_GamepadRemoved(GamepadRemovedToken);
    }
    if (RawGameControllerStatics) {
        RawGameControllerStatics->remove_RawGameControllerAdded(RawGameControllerAddedToken);
        RawGameControllerStatics->remove_RawGameControllerRemoved(RawGameControllerRemovedToken);
    }
    // now clear
    GamepadStatics = null;
    GamepadStatics2 = null;
    RawGameControllerStatics = null;
#endif

#if JOYPAD_THREAD
    JoypadThread.stop();
    JoypadEvent.on();
    JoypadThread.del(); // delete the thread first
    ThreadInputs.del();
#if WINDOWS_OLD
    if (TimerRes)
        timeEndPeriod(TimerRes);
#endif
#endif

    Joypads._data.del();
    if (auto func = App.joypad_changed)
        func();

#if MAC
    if (HidManager) {
        IOHIDManagerClose(HidManager, kIOHIDOptionsTypeNone);
        CFRelease(HidManager);
        HidManager = null;
    }
#endif
}
/******************************************************************************/
void JoypadsClass::acquire(Bool on) {
#if JOYPAD_THREAD
    SyncLocker lock(JoypadLock);
    if (on)
        JoypadThread.resume();
    else
        JoypadThread.pause();
    ThreadInputs.clear();
#endif
    REPAO(T).acquire(on);
}
void JoypadsClass::update() {
#if JOYPAD_THREAD
    if (elms()) {
        SyncLockerEx lock(JoypadLock, JoypadThread.active());
#else
    {
#endif
#if JOYPAD_THREAD
        REPAO(T).getState();
        if (ThreadInputs.elms()) {
            FREPA(ThreadInputs) // process in order
            {
                Input &input = ThreadInputs[i];
                DEBUG_ASSERT(input.type == INPUT_JOYPAD, "ThreadInputs isn't INPUT_JOYPAD");
                Joypad &joypad = T[input.device];
                if (IsDPad(input.jb)) {
                    // don't call 'setDiri', instead process manually more optimized
                    if (IsDPadY(input.jb)) {
                        if (input.pushed)
                            joypad.diri_r.y += (joypad.diri.y = (input.jb == JB_DPAD_UP ? 1 : -1));
                        else
                            joypad.diri.y = 0;
                    } else {
                        if (input.pushed)
                            joypad.diri_r.x += (joypad.diri.x = (input.jb == JB_DPAD_RIGHT ? 1 : -1));
                        else
                            joypad.diri.x = 0;
                    }
                    joypad.dir = joypad.diri;
                    if (joypad.diri.x && joypad.diri.y)
                        joypad.dir /= SQRT2; // dir.clipLength(1)
                    Inputs.add(input);
                } else {
                    if (input.pushed)
                        joypad.push(input.jb);
                    else
                        joypad.release(input.jb);
                }
            }
            ThreadInputs.clear();
        }
#endif
        REPAO(T).update(); // have to process at the end because we need final value of 'diri'
    }
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
