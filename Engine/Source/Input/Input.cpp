/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************/
#if !IS_POW_2(INPUT_COMBO_NUM)
#error INPUT_COMBO_NUM must be a power of 2
#endif
/******************************************************************************/
#if ANDROID
ASensorEventQueue *SensorEventQueue;
#elif IOS
CLLocationManager *LocationManager[2];
CMMotionManager *CoreMotionMgr;
CHHapticEngine *Vibrator;

static void DelVibrator() {
    [Vibrator release];
    Vibrator = nil;
}
#endif
Vec AccelerometerValue, GyroscopeValue, MagnetometerValue;
Bool LocationBackground[2];
Dbl LocationLat[2], LocationLon[2];
Flt LocationAlt[2], LocationAcc[2], LocationSpd[2], LocationInterval[2] = {-1, -1}, MagnetometerInterval = -1;
DateTime LocationTim[2];
InputDevicesClass InputDevices;
Memc<Input> Inputs;
/******************************************************************************/
// ACCELEROMETER, GYROSCOPE, MAGNETOMETER, LOCATION
/******************************************************************************/
C Vec &Accelerometer() { return AccelerometerValue; }
C Vec &Gyroscope() { return GyroscopeValue; }
C Vec &Magnetometer() { return MagnetometerValue; }

Dbl LocationLatitude(Bool gps) { return LocationLat[gps]; }
Dbl LocationLongitude(Bool gps) { return LocationLon[gps]; }
Flt LocationAltitude(Bool gps) { return LocationAlt[gps]; }
Flt LocationAccuracy(Bool gps) { return LocationAcc[gps]; }
Flt LocationSpeed(Bool gps) { return LocationSpd[gps]; }
C DateTime &LocationTimeUTC(Bool gps) { return LocationTim[gps]; }
/******************************************************************************/
#if !WINDOWS_NEW
void SetMagnetometerRefresh(Flt interval) {
#if ANDROID
    if (SensorEventQueue)
        if (ASensorManager *sensor_manager = ASensorManager_getInstance())
            if (C ASensor *magnetometer = ASensorManager_getDefaultSensor(sensor_manager, ASENSOR_TYPE_MAGNETIC_FIELD)) {
                if (interval >= 0) // enable
                {
                    ASensorEventQueue_setEventRate(SensorEventQueue, magnetometer, RoundU(interval * 1000000));
                    ASensorEventQueue_enableSensor(SensorEventQueue, magnetometer);
                } else
                    ASensorEventQueue_disableSensor(SensorEventQueue, magnetometer); // disable
            }
#elif IOS
    const Bool gps = true;
    if (CLLocationManager *lm = LocationManager[gps])
        if (interval >= 0 && [CLLocationManager headingAvailable]) {
            [lm startUpdatingHeading];
        } else {
            [lm stopUpdatingHeading];
        }
#endif
}
#endif
/******************************************************************************/
void SetLocationRefresh(Flt interval, Bool gps) {
#if ANDROID
    JNI jni;
    if (jni && EsenthelLocationListener[gps]) {
        if (interval >= 0) // enable
        {
            RequirePermission(PERMISSION_LOCATION);

            ULong min_time = RoundU(interval * 1000);
            jni->CallVoidMethod(LocationManager, requestLocationUpdates, gps ? GPS_PROVIDER() : NETWORK_PROVIDER(), min_time, 0.0f, EsenthelLocationListener[gps]());
        } else // disable
        {
            jni->CallVoidMethod(LocationManager, removeUpdates, EsenthelLocationListener[gps]());
        }
        if (jni->ExceptionCheck()) {
#if DEBUG
            jni->ExceptionDescribe();
#endif
            jni->ExceptionClear();
        }
    }
#elif IOS
    if (CLLocationManager *lm = LocationManager[gps])
        if (interval >= 0 && [CLLocationManager locationServicesEnabled]) {
            if (LocationBackground[gps])
                [lm requestAlwaysAuthorization];
            else
                [lm requestWhenInUseAuthorization];
            if (gps)
                [lm startUpdatingLocation];
            else
                [lm startMonitoringSignificantLocationChanges];
        } else {
            if (gps)
                [lm stopUpdatingLocation];
            else
                [lm stopMonitoringSignificantLocationChanges];
        }
#endif
}
/******************************************************************************/
void MagnetometerRefresh(Flt interval) {
    MAX(interval, 0);
    if (!Equal(MagnetometerInterval, interval))
        SetMagnetometerRefresh(MagnetometerInterval = interval);
}
void MagnetometerDisable() {
    if (!Equal(MagnetometerInterval, -1))
        SetMagnetometerRefresh(MagnetometerInterval = -1);
}

void LocationRefresh(Flt interval, Bool gps, Bool background) {
    MAX(interval, 0);
    if (!Equal(LocationInterval[gps], interval) || LocationBackground[gps] != background) {
        LocationBackground[gps] = background;
        SetLocationRefresh(LocationInterval[gps] = interval, gps);
    }
}
void LocationDisable(Bool gps) {
    if (!Equal(LocationInterval[gps], -1))
        SetLocationRefresh(LocationInterval[gps] = -1, gps);
}

LOCATION_AUTHORIZATION_STATUS LocationAuthorizationStatus() {
#if ANDROID
    return HasPermission(PERMISSION_LOCATION) ? LAS_BACKGROUND : LAS_DENIED;
#elif IOS
    switch ([CLLocationManager authorizationStatus]) {
    case kCLAuthorizationStatusRestricted:
        return LAS_RESTRICTED;
    case kCLAuthorizationStatusDenied:
        return LAS_DENIED;
    case kCLAuthorizationStatusAuthorizedWhenInUse:
        return LAS_FOREGROUND;
    case kCLAuthorizationStatusAuthorizedAlways:
        return LAS_BACKGROUND;
    }
#endif
    return LAS_UNKNOWN;
}
/******************************************************************************/
#if ANDROID
static ULong LocationTime[2];
void UpdateLocation(jobject location, Bool gps, JNI &jni) {
    if (location && jni && getTime) {
        ULong time = jni->CallLongMethod(location, getTime); // milliseconds since 1st Jan 1970
        if (time != LocationTime[gps]) {
            LocationTim[gps].from1970ms(LocationTime[gps] = time);
            LocationLat[gps] = jni->CallDoubleMethod(location, getLatitude);
            LocationLon[gps] = jni->CallDoubleMethod(location, getLongitude);
            LocationAlt[gps] = jni->CallDoubleMethod(location, getAltitude);
            LocationAcc[gps] = jni->CallFloatMethod(location, getAccuracy);
            LocationSpd[gps] = jni->CallFloatMethod(location, getSpeed);
        }
    }
}
void UpdateLocation(Bool gps, JNI &jni) {
    if (jni && getLastKnownLocation) {
        JObject location = JObject(jni, jni->CallObjectMethod(LocationManager, getLastKnownLocation, gps ? GPS_PROVIDER() : NETWORK_PROVIDER()));
        if (jni->ExceptionCheck()) {
#if DEBUG
            jni->ExceptionDescribe();
#endif
            location.clear();
            jni->ExceptionClear();
        }
        UpdateLocation(location, gps, jni);
    }
}
void UpdateLocation(JNI &jni) {
    REP(2)
    UpdateLocation(i != 0, jni);
}
#endif
/******************************************************************************/
void DeviceVibrate(Flt intensity, Flt duration) {
#if ANDROID
    if (Jni && ActivityClass && vibrate)
        if (Byte int_byte = FltToByte(intensity)) // 0 will crash
        {
            Int milliseconds = RoundPos(duration * 1000);
            if (milliseconds > 0)
                Jni->CallStaticVoidMethod(ActivityClass, vibrate, jint(int_byte), jint(milliseconds)); // <0 will crash, =0 is useless
        }
#elif IOS
    if (Vibrator && intensity > 0) {
        intensity = SqrtFast(intensity); // sqrt because on iOS small vibrations are barely noticable
#if 0
      CHHapticEventParameter *param=[[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticIntensity value:intensity];
      CHHapticEvent *event=[[CHHapticEvent alloc] initWithEventType:CHHapticEventTypeHapticContinuous parameters:[NSArray arrayWithObjects:param, nil] relativeTime:0 duration:duration];
      NSError *error=nil; CHHapticPattern *pattern=[[CHHapticPattern alloc] initWithEvents:[NSArray arrayWithObject:event] parameters:[[NSArray alloc] init] error:&error]; if(!error)
         if(auto player=[Vibrator createPlayerWithPattern:pattern error:&error])
      {
         if(!error)[player startAtTime:0 error:&error]; // don't set "error:nil" because if error happens then exception occurs with nil
       //[player release]; don't release, it will crash
      }
#else
        NSDictionary *dict =
            @{
                CHHapticPatternKeyPattern :
                    @[
                        @{
                            CHHapticPatternKeyEvent :
                                @{
                                    CHHapticPatternKeyEventType : CHHapticEventTypeHapticContinuous,
                                    CHHapticPatternKeyTime : @(CHHapticTimeImmediate),
                                    CHHapticPatternKeyEventDuration : @(duration),
                                    CHHapticPatternKeyEventParameters :
                                        @[
                                            @{
                                                CHHapticPatternKeyParameterID : CHHapticEventParameterIDHapticIntensity,
                                                CHHapticPatternKeyParameterValue : @(intensity),
                                            },
                                        ],
                                },
                        },
                    ],
            };
        NSError *error = nil;
        CHHapticPattern *pattern = [[CHHapticPattern alloc] initWithDictionary:dict error:&error];
        if (!error)
            if (auto player = [Vibrator createPlayerWithPattern:pattern error:&error]) {
                if (!error)
                    [player startAtTime:0 error:&error]; // don't set "error:nil" because if error happens then exception occurs with nil
                                                         //[player release]; don't release, it will crash
            }
#endif
    }
#endif
}
void DeviceVibrateShort() { DeviceVibrate(1, 0.02f); }
/******************************************************************************/
// INPUT
/******************************************************************************/
void InputDevicesClass::del() {
    ShutJoypads();
    Ms.del();
    Kb.del();
#if WINDOWS_OLD && DIRECT_INPUT
    RELEASE(DI);
#elif IOS
    [CoreMotionMgr release];
    CoreMotionMgr = nil;
    DelVibrator();
#endif
    VR.shut(); // !! delete as last, after the mouse, because it may try to reset the mouse cursor, so we need to make sure that mouse cursor was already deleted !!
}
/******************************************************************************/
void InputDevicesClass::checkMouseKeyboard() {
#if WINDOWS_OLD && (KB_RAW_INPUT || MS_RAW_INPUT)
    if (KB_RAW_INPUT)
        Kb._hardware = false;
    if (MS_RAW_INPUT)
        Ms._hardware = false;
    Memt<RAWINPUTDEVICELIST> devices;
    UINT num_devices = 0;
    GetRawInputDeviceList(null, &num_devices, SIZE(RAWINPUTDEVICELIST));
again:
    devices.setNum(num_devices);
    Int out = GetRawInputDeviceList(devices.data(), &num_devices, SIZE(RAWINPUTDEVICELIST));
    if (out < 0) // error
    {
        if (Int(num_devices) > devices.elms())
            goto again; // need more memory
        devices.clear();
    } else {
        if (out < devices.elms())
            devices.setNum(out);
        FREPA(devices) {
            C RAWINPUTDEVICELIST &device = devices[i];
            switch (device.dwType) {
#if KB_RAW_INPUT
            case RIM_TYPEKEYBOARD:
                Kb._hardware = true;
                break;
#endif
#if MS_RAW_INPUT
            case RIM_TYPEMOUSE:
                Ms._hardware = Ms._detected = true;
                break; // don't set false in case user called 'Ms.simulate'
#endif
            }
            /*UInt size=0; if(Int(GetRawInputDeviceInfoW(device.hDevice, RIDI_DEVICENAME, null, &size))>=0)
              {
                 Memt<Char> name; name.setNum(size+1); Int r=GetRawInputDeviceInfoW(device.hDevice, RIDI_DEVICENAME, name.data(), &size);
                 if(r>=0 && size==r && r+1==name.elms())
                 {
                    name.last()='\0'; // in case it's needed
                    Str n=name.data();
                 }
              }*/
        }
    }
#elif WINDOWS_NEW
    // use try/catch because exceptions can occur, something about "data is too small"
    try {
        Kb._hardware = (Windows::Devices::Input::KeyboardCapabilities().KeyboardPresent > 0);
    } catch (...) {
    }
    try {
        Ms._hardware = (Windows::Devices::Input::MouseCapabilities().MousePresent > 0);
        if (Ms._hardware)
            Ms._detected = true;
    } catch (...) {
    } // don't set false in case user called 'Ms.simulate'
#elif ANDROID
    Kb._hardware = ((AndroidApp && AndroidApp->config) ? FlagOn(AConfiguration_getKeyboard(AndroidApp->config), (UInt)ACONFIGURATION_KEYBOARD_QWERTY) : false);
#endif
}
/******************************************************************************/
void InputDevicesClass::create() {
    if (LogInit)
        LogN("InputDevicesClass.create");
#if WINDOWS_OLD
    if (LogInit)
        LogN("DirectInput8Create");
#if DIRECT_INPUT
    DYNAMIC_ASSERT(OK(DirectInput8Create(App._hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (Ptr *)&DI, null)), "Can't create DirectInput");
#endif
#endif
    checkMouseKeyboard();
    Kb.create();
    Ms.create();
    InitJoypads();
    // VR  .init  (); this is now to be called manually in 'InitPre'
#if IOS
    Int hz = (D.freq() > 0 ? D.freq() : 60); // default to 60 events per second
    Flt interval = 1.0f / hz;
    CoreMotionMgr = [[CMMotionManager alloc] init];
    CoreMotionMgr.accelerometerUpdateInterval = interval;
    CoreMotionMgr.gyroUpdateInterval = interval;

    // Vibrator
    if ([CHHapticEngine class]) // if class was found, this is needed because CoreHaptics is linked optionally as "weak_framework CoreHaptics"
        if (CHHapticEngine.capabilitiesForHardware.supportsHaptics) {
            NSError *error = nil;
            Vibrator = [[CHHapticEngine alloc] initAndReturnError:&error];
            if (error)
                DelVibrator();
            else {
                [Vibrator setResetHandler:^{
                  NSError *error = nil;
                  [Vibrator startAndReturnError:&error]; // don't set "error:nil" because if error happens then exception occurs with nil
                }];
            }
        }
#elif SWITCH
    NS::CreateInput();
#endif
    if (App.active())
        acquire(true);
}
/******************************************************************************/
void InputDevicesClass::update() {
#if SWITCH
    NS::UpdateInput();
#endif
    Kb.update();
    Ms.update();
    if (App.active())
        Joypads.update();
    TouchesUpdate();
    VR.update();
#if ANDROID
    if (!(Time.frame() % 60))
        UpdateLocation(Jni); // update once per 60-frames, because it was reported that this can generate plenty of messages in the log
#elif IOS
    if (CoreMotionMgr) {
        if (CMAccelerometerData *acceleration = CoreMotionMgr.accelerometerData)
            AccelerometerValue.set(acceleration.acceleration.x, acceleration.acceleration.y, -acceleration.acceleration.z) *= 9.80665f;
        if (CMGyroData *gyroscope = CoreMotionMgr.gyroData)
            GyroscopeValue.set(gyroscope.rotationRate.x, gyroscope.rotationRate.y, -gyroscope.rotationRate.z);
    }
#endif
}
void InputDevicesClass::clear() {
    Kb.clear();
    Ms.clear();
    if (App.active())
        REPAO(Joypads).clear();
    TouchesClear();
    Inputs.clear();
}
/******************************************************************************/
void InputDevicesClass::acquire(Bool on) {
    Kb.acquire(on);
    Ms.acquire(on);
    Joypads.acquire(on);
#if ANDROID
    if (SensorEventQueue)
        if (ASensorManager *sensor_manager = ASensorManager_getInstance()) {
            Int hz = (D.freq() > 0 ? D.freq() : 60); // default to 60 events per second
            Int microseconds = 1000 * 1000 / hz;
            if (C ASensor *accelerometer = ASensorManager_getDefaultSensor(sensor_manager, ASENSOR_TYPE_ACCELEROMETER)) {
                if (on) // start monitoring the accelerometer
                {
                    ASensorEventQueue_setEventRate(SensorEventQueue, accelerometer, microseconds);
                    ASensorEventQueue_enableSensor(SensorEventQueue, accelerometer);
                } else
                    ASensorEventQueue_disableSensor(SensorEventQueue, accelerometer); // stop monitoring accelerometer
            }
            if (C ASensor *gyroscope = ASensorManager_getDefaultSensor(sensor_manager, ASENSOR_TYPE_GYROSCOPE)) {
                if (on) // start monitoring the gyroscope
                {
                    ASensorEventQueue_setEventRate(SensorEventQueue, gyroscope, microseconds);
                    ASensorEventQueue_enableSensor(SensorEventQueue, gyroscope);
                } else
                    ASensorEventQueue_disableSensor(SensorEventQueue, gyroscope); // stop monitoring gyroscope
            }
        }
    if (on)
        UpdateLocation(Jni);
#elif IOS
    if (Vibrator) // have to stop and restart on app pause/resume, without this vibrations will stop playing
    {
        if (on) {
            NSError *error = nil;
            [Vibrator startAndReturnError:&error];
        } // don't set "error:nil" because if error happens then exception occurs with nil
        else
            [Vibrator stopWithCompletionHandler:nil];
    }
    if (CoreMotionMgr) {
        if (on) {
            [CoreMotionMgr startAccelerometerUpdates];
            [CoreMotionMgr startGyroUpdates];
        } else {
            [CoreMotionMgr stopAccelerometerUpdates];
            [CoreMotionMgr stopGyroUpdates];
        }
    }
#endif
    REPA(LocationInterval)
    SetLocationRefresh(on ? LocationInterval[i] : -1, i != 0);
    SetMagnetometerRefresh(on ? MagnetometerInterval : -1);
}
/******************************************************************************/
// TEXT EDIT
/******************************************************************************/
static void TextSelCopy(C Str &str, TextEdit &edit) // assumes that 'sel' and 'cur' are in 'str' range (or sel<0)
{
    if (edit.sel < 0)
        ClipSet(str);
    else                                           // copy all
        if (Int length = Abs(edit.sel - edit.cur)) // copy part
        {
            Memt<Char> temp;
            temp.setNum(length + 1);
            CopyFastN(temp.data(), str() + Min(edit.sel, edit.cur), length);
            temp[length] = 0;
            ClipSet(temp.data());
        } else
            ClipSet(S); // copy nothing
}
static Bool TextSelRem(Str &str, TextEdit &edit) // assumes that 'sel' and 'cur' are in 'str' range (or sel<0)
{
    if (edit.sel < 0) // remove all
    {
        str.clear();
        edit.cur = 0;
        edit.sel = -1;
    } else // remove part
    {
        Int pos = Min(edit.sel, edit.cur),
            cut = Abs(edit.sel - edit.cur);
        str.remove(pos, cut);
        edit.cur = pos;
        edit.sel = -1;
    }
    return true;
}
static void SkipLeft(C Str &str, TextEdit &edit) {
    for (; CharFlagFast(str[edit.cur]) & CHARF_SKIP && edit.cur > 0;)
        edit.cur--;
}
static void SkipRight(C Str &str, TextEdit &edit) {
    for (; CharFlagFast(str[edit.cur]) & CHARF_SKIP;)
        edit.cur++;
}
static Bool Processed(Str &str, TextEdit &edit, Bool multi_line, C KeyboardKey &key, Bool &changed) {
    if (!key.winCtrl()) {
        switch (key.k) {
        case KB_HOME: {
            if (!key.shift())
                edit.sel = -1;
            else if (edit.sel < 0)
                edit.sel = edit.cur;
            if (!multi_line || key.ctrlCmd())
                edit.cur = 0;
            else
                for (; edit.cur > 0 && str[edit.cur - 1] != '\n';)
                    edit.cur--;
            return true;
        } break;

        case KB_END: {
            if (!key.shift())
                edit.sel = -1;
            else if (edit.sel < 0)
                edit.sel = edit.cur;
            if (!multi_line || key.ctrlCmd())
                edit.cur = str.length();
            else
                for (; edit.cur < str.length() && str[edit.cur] != '\n';)
                    edit.cur++;
            return true;
        } break;

        case KB_LEFT: {
            if (edit.sel >= 0 && !key.shift()) {
                edit.cur = Min(edit.cur, edit.sel);
                edit.sel = -1;
            } else // cancel selection
                if (edit.cur) {
                    if (key.shift() && edit.sel < 0)
                        edit.sel = edit.cur; // start selection from cursor
                    edit.cur--;
                    SkipLeft(str, edit);
                    if (key.ctrlCmd())
                        if (edit.password)
                            edit.cur = 0;
                        else
                            for (CHAR_TYPE ct = CharType(str[edit.cur]); edit.cur;) {
                                CHAR_TYPE nt = CharType(str[edit.cur - 1]);
                                if (ct == CHART_SPACE)
                                    ct = nt;
                                if (ct != nt)
                                    break;
                                edit.cur--;
                                SkipLeft(str, edit);
                            }
                }
            return true;
        } break;

        case KB_RIGHT: {
            if (edit.sel >= 0 && !key.shift()) {
                edit.cur = Max(edit.cur, edit.sel);
                edit.sel = -1;
            } else // cancel selection
                if (edit.cur < str.length()) {
                    if (key.shift() && edit.sel < 0)
                        edit.sel = edit.cur; // start selection from cursor
                    edit.cur++;
                    SkipRight(str, edit);
                    if (key.ctrlCmd())
                        if (edit.password)
                            edit.cur = str.length();
                        else
                            for (CHAR_TYPE ct = CharType(str[edit.cur - 1]); edit.cur < str.length();) {
                                CHAR_TYPE nt = CharType(str[edit.cur]);
                                if (ct != nt) {
                                    for (; edit.cur < str.length() && str[edit.cur] == ' ';)
                                        edit.cur++;
                                    break;
                                }
                                edit.cur++;
                                SkipRight(str, edit);
                            }
                }
            return true;
        } break;

        case KB_BACK: {
            if (edit.sel >= 0)
                changed |= TextSelRem(str, edit);
            else if (edit.cur) {
                if (key.ctrlCmd()) {
                    edit.sel = edit.cur;
                    if (edit.password)
                        edit.cur = 0;
                    else {
                        edit.cur--;
                        for (CHAR_TYPE ct = CharType(str[edit.cur]); edit.cur; edit.cur--) {
                            CHAR_TYPE nt = CharType(str[edit.cur - 1]);
                            if (ct != nt)
                                break;
                        }
                    }
                    TextSelRem(str, edit);
                } else {
                    Int end = edit.cur--;
                    for (; CharFlagFast(str[edit.cur]) & CHARF_MULTI1 && edit.cur > 0;)
                        edit.cur--; // here check only MULTI1 to delete full multi-character, don't check CHARF_COMBINING to allow removing only combining while leaving base and other combining
                    str.remove(edit.cur, end - edit.cur);
                }
                changed = true;
            }
            return true;
        } break;

        case KB_TAB:
            if (multi_line && !key.alt()) {
                if (edit.sel >= 0)
                    changed |= TextSelRem(str, edit);
                /*if(edit.overwrite && edit.cur<str.length())str.    _d[edit.cur]='\t';
                  else*/
                str.insert(edit.cur, '\t');
                edit.cur++;
                changed = true;
                return true;
            }
            break;

        case KB_ENTER:
            if (multi_line || key.shift() && !key.ctrl() && !key.alt()) // allow adding new line only in multi_line 'TextBox', or 'TextLine' too but only if Shift is pressed without Ctrl/Alt
            {
                if (edit.sel >= 0)
                    changed |= TextSelRem(str, edit);
                /*if(edit.overwrite && edit.cur<str.length())str.    _d[edit.cur]='\n';
                  else*/
                str.insert(edit.cur, '\n');
                edit.cur++;
                changed = true;
                return true;
            }
            break;

        case KB_INS: {
            if (key.ctrlCmd()) // copy
            {
            copy:
                if (!edit.password)
                    TextSelCopy(str, edit);
            } else if (key.shift()) // paste
            {
            paste:
                if (edit.sel >= 0)
                    changed |= TextSelRem(str, edit);
                Str clip = ClipGet();
                if (clip.is()) {
                    str.insert(edit.cur, clip);
                    edit.cur += clip.length();
                    changed = true;
                }
            } else
                edit.overwrite ^= 1;
            return true;
        } break;

        case KB_DEL: {
            if (key.shift()) // cut
            {
            cut:
                if (!edit.password) {
                    TextSelCopy(str, edit);
                    changed |= TextSelRem(str, edit);
                }
            } else {
                if (edit.sel >= 0)
                    changed |= TextSelRem(str, edit);
                else if (edit.cur < str.length()) {
                    if (key.ctrlCmd()) {
                        edit.sel = edit.cur;
                        if (edit.password)
                            edit.cur = str.length();
                        else {
                            edit.cur++;
                            for (CHAR_TYPE ct = CharType(str[edit.cur - 1]); edit.cur < str.length(); edit.cur++) {
                                CHAR_TYPE nt = CharType(str[edit.cur]);
                                if (ct == CHART_SPACE)
                                    ct = nt;
                                if (ct != nt) {
                                    for (; edit.cur < str.length() && str[edit.cur] == ' ';)
                                        edit.cur++;
                                    break;
                                }
                            }
                        }
                        TextSelRem(str, edit);
                    } else {
                        Int num = 1;
                        for (; CharFlagFast(str[edit.cur + num]) & CHARF_SKIP;)
                            num++; // del all combining characters after deleted one
                        str.remove(edit.cur, num);
                    }
                    changed = true;
                }
            }
            return true;
        } break;

        case KB_C:
            if (key.ctrlCmd())
                goto copy;
            break;
        case KB_V:
            if (key.ctrlCmd())
                goto paste;
            break;
        case KB_X:
            if (key.ctrlCmd())
                goto cut;
            break;

        case KB_A:
            if (key.ctrlCmd()) {
                if (str.length()) {
                    edit.sel = 0;
                    edit.cur = str.length();
                }
                return true;
            }
            break;
        }

        if (!key.ctrlCmd() && !key.lalt())
            if (key.c) {
                if (edit.sel >= 0)
                    changed |= TextSelRem(str, edit);
                if (edit.overwrite && edit.cur < str.length()) {
                    Int num = 0;
                    for (; CharFlagFast(str[edit.cur + 1 + num]) & CHARF_SKIP;)
                        num++;
                    str.remove(edit.cur + 1, num); // del all combining characters after replaced one
                    str._d[edit.cur] = key.c;
                } else
                    str.insert(edit.cur, key.c);
                edit.cur++;
                changed = true;
                return true;
            }
    }
    return false;
}
Bool EditText(Str &str, TextEdit &edit, Bool multi_line) {
    Bool changed = false;
    if (edit.cur > str.length() || edit.cur < 0)
        edit.cur = str.length();
    if (edit.sel > str.length())
        edit.sel = str.length();

    if (Ms.bp(MS_BACK)) {
        KeyboardKey key;
        key.clear();
        key.k = KB_BACK;
        Processed(str, edit, multi_line, key, changed);
        Ms.eat(4);
    }

    if (Processed(str, edit, multi_line, Kb.k, changed)) {
    again:
        Kb.eatKey();                                             // eat only if processed
        if (C KeyboardKey *key = Kb.nextKeyPtr())                // get next key without moving it
            if (Processed(str, edit, multi_line, *key, changed)) // if can process this key, then eat it, otherwise, leave it to appear in the next frame so other codes before 'EditText' can try to process it
            {
                Kb.nextInQueue(); // move 'key' to 'Kb.k'
                goto again;
            }
    }

    if (edit.sel == edit.cur)
        edit.sel = -1;
    return changed;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
