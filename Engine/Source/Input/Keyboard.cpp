/******************************************************************************/
#include "../../stdafx.h"
#if IOS
#include "../Platforms/iOS/iOS.h"
#endif
namespace EE {
/******************************************************************************

   Operate on Scan Codes as Int, valid >=0, invalid <0, because on:
      Mac KB_A has scan code=0
      Android there are scan codes bigger than byte

/******************************************************************************/
#if WINDOWS_OLD

/*
   RightAlt (AltGr on Polish, Norwegian, .. keyboards) also triggers
   GetKeyState/GetAsyncKeyState VK_LCONTROL changing Direct Input exclusive mode
   (calling 'Unacquire' and 'Acquire') resets the 'GetDeviceState' of any
   currently pressed keys, they need to be pushed again to activate their state
   Pressing Ctrl+Shift (system shortcut for changing language input) may disable
   'GetKeyState' for Ctrl Prefer key events instead of 'GetDeviceState' to
   record pushes/releases within the same frame
*/

enum COOP_MODE {
    BACKGROUND,
    FOREGROUND,
    KEYBOARD_MODE =
        FOREGROUND, // prefer FOREGROUND to avoid recording input when not
                    // focused, maybe BACKGROUND could trigger some anti-virus
                    // warnings (potential key-logger), otherwise we could use
                    // (_exclusive ? FOREGROUND : BACKGROUND) // prefer background
                    // mode so we can get correct information about
                    // 'GetDeviceState' when activating the app with LMB
                    // (otherwise, the state is delayed by 1 frame), however
                    // _exclusive does not support BACKGROUND
};

#if KB_DIRECT_INPUT
#define BUF_KEYS 256
struct DIK {
    KB_KEY key;
    Byte dik, scan_code;
};
static const DIK Keys[] = // only keys known to have the same physical location
                          // on all keyboard layouts can be listed here
    {
        {KB_LCTRL, DIK_LCONTROL, 29},
        {KB_RCTRL, DIK_RCONTROL, 29},
        {KB_LSHIFT, DIK_LSHIFT, 42}, // WM_*KEY* does not provide an option to
                                     // check for left/right shift
        {KB_RSHIFT, DIK_RSHIFT, 54}, // WM_*KEY* does not provide an option to
                                     // check for left/right shift
        //{KB_LALT  , DIK_LALT    , 56}, processed using WM_*KEY*
        //{KB_RALT  , DIK_RALT    , 56}, processed using WM_*KEY*
        //{KB_LWIN  , DIK_LWIN    , 91}, processed using WM_*KEY*
        //{KB_RWIN  , DIK_RWIN    , 92}, processed using WM_*KEY*
        {KB_PRINT, DIK_SYSRQ, 0}, // VK_PRINT is not processed because it's
                                  // assigned as Screen Capture
};
static Byte _special;
#endif
#elif WINDOWS_NEW
Windows::UI::Text::Core::CoreTextEditContext ^ TextEditContext;
#elif ANDROID
/*
Key characters come from:
   -hardware keyboards (detected only from C++ based on keycodes,
'dispatchKeyEvent' does not detect them) -touch    keyboards (detected      from
C++ based on keycodes but without unicode and from 'dispatchKeyEvent') To
prevent accidental detecting the same key twice, only one source is allowed per
frame.
*/
struct InputText {
    Str text;
    VecI2 cur;
};
static Bool InputTextIs;
static InputText InputTextData;
static SyncLock InputTextLock;
Byte KeySource;
#endif
KeyboardClass Kb;
/******************************************************************************/
#if ANDROID || SWITCH
static Char8 _key_char_shift[ELMS(Kb._key_char)];
#endif
inline static void Set(KB_KEY key, Char8 c, Char8 qwerty_shift, CChar8 *name,
                       CChar *symbol) {
    Kb._key_char[key] = c;
    Kb._key_name[key] = name;
    Kb._key_symbol[key] = symbol;
#if ANDROID || SWITCH
    _key_char_shift[key] = qwerty_shift;
#endif
}
KeyboardClass::KeyboardClass() {
#if 0 // there's only one 'KeyboardClass' global 'Kb' and it doesn't need \
      // clearing members to zero
  _exclusive=_visible=_hardware=false;
  _device=null;
  _imc=null;
   REPA(key_char)Set(KB_KEY(i), '\0', '\0', null);
   ..
#endif
    _last_key_scan_code = -1;
    _imm = true;
    _cur = _last = -1;
    _last_t = 0;
    _curh_tn = 0.200f;

    // set these first in case KB_NPENTER==KB_ENTER, so that KB_ENTER can override
    // the name of KB_NPENTER
    Set(KB_NPDIV, '/', '/', "Numpad /", u"Numpad/");
    Set(KB_NPMUL, '*', '*', "Numpad *", u"Numpad*");
    Set(KB_NPSUB, '-', '-', "Numpad -", u"Numpad-");
    Set(KB_NPADD, '+', '+', "Numpad +", u"Numpad+");
    Set(KB_NPDEL, '\0', '\0', "Numpad Del", u"NumpadDel");
    Set(KB_NPENTER, '\n', '\n', "Numpad Enter", u"NumpadEnter");

    Set(KB_NP0, '0', '\0', "Numpad 0", u"Numpad0");
    Set(KB_NP1, '1', '\0', "Numpad 1", u"Numpad1");
    Set(KB_NP2, '2', '\0', "Numpad 2", u"Numpad2");
    Set(KB_NP3, '3', '\0', "Numpad 3", u"Numpad3");
    Set(KB_NP4, '4', '\0', "Numpad 4", u"Numpad4");
    Set(KB_NP5, '5', '\0', "Numpad 5", u"Numpad5");
    Set(KB_NP6, '6', '\0', "Numpad 6", u"Numpad6");
    Set(KB_NP7, '7', '\0', "Numpad 7", u"Numpad7");
    Set(KB_NP8, '8', '\0', "Numpad 8", u"Numpad8");
    Set(KB_NP9, '9', '\0', "Numpad 9", u"Numpad9");

    Set(KB_0, '0', ')', "0", u"0");
    Set(KB_1, '1', '!', "1", u"1");
    Set(KB_2, '2', '@', "2", u"2");
    Set(KB_3, '3', '#', "3", u"3");
    Set(KB_4, '4', '$', "4", u"4");
    Set(KB_5, '5', '%', "5", u"5");
    Set(KB_6, '6', '^', "6", u"6");
    Set(KB_7, '7', '&', "7", u"7");
    Set(KB_8, '8', '*', "8", u"8");
    Set(KB_9, '9', '(', "9", u"9");

    Set(KB_A, 'a', 'A', "A", u"A");
    Set(KB_B, 'b', 'B', "B", u"B");
    Set(KB_C, 'c', 'C', "C", u"C");
    Set(KB_D, 'd', 'D', "D", u"D");
    Set(KB_E, 'e', 'E', "E", u"E");
    Set(KB_F, 'f', 'F', "F", u"F");
    Set(KB_G, 'g', 'G', "G", u"G");
    Set(KB_H, 'h', 'H', "H", u"H");
    Set(KB_I, 'i', 'I', "I", u"I");
    Set(KB_J, 'j', 'J', "J", u"J");
    Set(KB_K, 'k', 'K', "K", u"K");
    Set(KB_L, 'l', 'L', "L", u"L");
    Set(KB_M, 'm', 'M', "M", u"M");
    Set(KB_N, 'n', 'N', "N", u"N");
    Set(KB_O, 'o', 'O', "O", u"O");
    Set(KB_P, 'p', 'P', "P", u"P");
    Set(KB_Q, 'q', 'Q', "Q", u"Q");
    Set(KB_R, 'r', 'R', "R", u"R");
    Set(KB_S, 's', 'S', "S", u"S");
    Set(KB_T, 't', 'T', "T", u"T");
    Set(KB_U, 'u', 'U', "U", u"U");
    Set(KB_V, 'v', 'V', "V", u"V");
    Set(KB_W, 'w', 'W', "W", u"W");
    Set(KB_X, 'x', 'X', "X", u"X");
    Set(KB_Y, 'y', 'Y', "Y", u"Y");
    Set(KB_Z, 'z', 'Z', "Z", u"Z");

    Set(KB_F1, '\0', '\0', "F1", u"F1");
    Set(KB_F2, '\0', '\0', "F2", u"F2");
    Set(KB_F3, '\0', '\0', "F3", u"F3");
    Set(KB_F4, '\0', '\0', "F4", u"F4");
    Set(KB_F5, '\0', '\0', "F5", u"F5");
    Set(KB_F6, '\0', '\0', "F6", u"F6");
    Set(KB_F7, '\0', '\0', "F7", u"F7");
    Set(KB_F8, '\0', '\0', "F8", u"F8");
    Set(KB_F9, '\0', '\0', "F9", u"F9");
    Set(KB_F10, '\0', '\0', "F10", u"F10");
    Set(KB_F11, '\0', '\0', "F11", u"F11");
    Set(KB_F12, '\0', '\0', "F12", u"F12");

    Set(KB_ESC, '\0', '\0', "Escape", u"Esc");
    Set(KB_ENTER, '\n', '\n', "Enter", u"Enter");
    Set(KB_SPACE, ' ', ' ', "Space", u"Space");
    Set(KB_BACK, '\0', '\0', "Backspace", u"Back");
    Set(KB_TAB, '\t', '\t', "Tab", u"Tab");

    Set(KB_CTRL, '\0', '\0', "Control", u"Ctrl");
    Set(KB_SHIFT, '\0', '\0', "Shift", u"Shift");
    Set(KB_ALT, '\0', '\0', "Alt", u"Alt");

    Set(KB_LCTRL, '\0', '\0', "Left Control", u"LeftCtrl");
    Set(KB_RCTRL, '\0', '\0', "Right Control", u"RightCtrl");
    Set(KB_LSHIFT, '\0', '\0', "Left Shift", u"LeftShift");
    Set(KB_RSHIFT, '\0', '\0', "Right Shift", u"RightShift");
    Set(KB_LALT, '\0', '\0', "Left Alt", u"LeftAlt");
    Set(KB_RALT, '\0', '\0', "Right Alt", u"RightAlt");
#if APPLE
    Set(KB_LWIN, '\0', '\0', "Left Cmd", u"LeftCmd");
    Set(KB_RWIN, '\0', '\0', "Right Cmd", u"RightCmd");
#else
    Set(KB_LWIN, '\0', '\0', "Left Win", u"LeftWin");
    Set(KB_RWIN, '\0', '\0', "Right Win", u"RightWin");
#endif
    Set(KB_MENU, '\0', '\0', "Menu", u"Menu");
    Set(KB_FIND, '\0', '\0', "Find", u"Find");

    Set(KB_LEFT, '\0', '\0', "Left", u"⯇");
    Set(KB_RIGHT, '\0', '\0', "Right", u"⯈");
    Set(KB_UP, '\0', '\0', "Up", u"⯅");
    Set(KB_DOWN, '\0', '\0', "Down", u"⯆");

    Set(KB_INS, '\0', '\0', "Insert", u"Ins");
    Set(KB_DEL, '\0', '\0', "Delete", u"Del");
    Set(KB_HOME, '\0', '\0', "Home", u"Home");
    Set(KB_END, '\0', '\0', "End", u"End");
    Set(KB_PGUP, '\0', '\0', "Page Up", u"PageUp");
    Set(KB_PGDN, '\0', '\0', "Page Down", u"PageDn");

    Set(KB_SUB, '-', '_', "Minus", u"-");
    Set(KB_EQUAL, '=', '+', "Equal", u"=");
    Set(KB_LBR, '[', '{', "Left Bracket", u"[");
    Set(KB_RBR, ']', '}', "Right Bracket", u"]");
    Set(KB_SEMICOLON, ';', ':', "Semicolon", u";");
    Set(KB_APOSTROPHE, '\'', '"', "Apostrophe", u"'");
    Set(KB_COMMA, ',', '<', "Comma", u",");
    Set(KB_DOT, '.', '>', "Dot", u".");
    Set(KB_SLASH, '/', '?', "Slash", u"/");
    Set(KB_BACKSLASH, '\\', '|', "Backslash", u"\\");
    Set(KB_TILDE, '`', '~', "Tilde", u"Tilde");

    Set(KB_CAPS, '\0', '\0', "Caps Lock", u"CapsLock");
    Set(KB_NUM, '\0', '\0', "Num Lock", u"NumLock");
    Set(KB_SCROLL, '\0', '\0', "Scroll Lock", u"ScrollLock");
    Set(KB_PRINT, '\0', '\0', "Print Screen", u"PrintScreen");
    Set(KB_PAUSE, '\0', '\0', "Pause", u"Pause");

    Set(KB_VOL_DOWN, '\0', '\0', "Volume Down", u"VolumeDown");
    Set(KB_VOL_UP, '\0', '\0', "Volume Up", u"VolumeUp");
    Set(KB_VOL_MUTE, '\0', '\0', "Volume Mute", u"VolumeMute");

    Set(KB_NAV_BACK, '\0', '\0', "Navigate Backward", u"NavigateBackward");
    Set(KB_NAV_FORWARD, '\0', '\0', "Navigate Forward", u"NavigateForward");

    Set(KB_MEDIA_PREV, '\0', '\0', "Media Previous", u"MediaPrevious");
    Set(KB_MEDIA_NEXT, '\0', '\0', "Media Next", u"MediaNext");
    Set(KB_MEDIA_PLAY, '\0', '\0', "Media Play", u"MediaPlay");
    Set(KB_MEDIA_STOP, '\0', '\0', "Media Stop", u"MediaStop");

    Set(KB_ZOOM_IN, '\0', '\0', "Zoom In", u"ZoomIn");
    Set(KB_ZOOM_OUT, '\0', '\0', "Zoom Out", u"ZoomOut");
}
#if MAC
static void KeyboardChanged(CFNotificationCenterRef center, void *observer,
                            CFStringRef name, const void *object,
                            CFDictionaryRef user_info) {
    Kb.setLayout();
}
#endif
void KeyboardClass::init() // Linux requires XDisplay to be created, so we can't
                           // do this in the constructor
{
    setLayout();
#if MAC
    // add callback when keyboard layout changes
    CFNotificationCenterAddObserver(
        CFNotificationCenterGetDistributedCenter(), null, KeyboardChanged,
        kTISNotifySelectedKeyboardInputSourceChanged, null,
        CFNotificationSuspensionBehaviorDeliverImmediately);
#endif
}
/******************************************************************************/
void KeyboardClass::del() {
#if WINDOWS_OLD
#if KB_RAW_INPUT
    RAWINPUTDEVICE rid[1];

    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x06; // keyboard
    rid[0].dwFlags = RIDEV_REMOVE;
    rid[0].hwndTarget = App.window();

    RegisterRawInputDevices(rid, Elms(rid), SIZE(RAWINPUTDEVICE));
#elif KB_DIRECT_INPUT
    RELEASE(_device);
#endif
#endif
}
void KeyboardClass::create() {
    if (LogInit)
        LogN("Keyboard.create");
#if WINDOWS_OLD
#if KB_RAW_INPUT
    RAWINPUTDEVICE rid[1];

    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x06; // keyboard
    rid[0].dwFlags = ((KEYBOARD_MODE == BACKGROUND) ? RIDEV_INPUTSINK : 0) |
                     (_exclusive ? RIDEV_NOHOTKEYS : 0);
    rid[0].hwndTarget = App.window();

    RegisterRawInputDevices(rid, Elms(rid), SIZE(RAWINPUTDEVICE));
    // detection is done in 'checkMouseKeyboard'
#elif KB_DIRECT_INPUT
    // DISCL_EXCLUSIVE|DISCL_BACKGROUND is not possible at all
    // DISCL_NOWINKEY |DISCL_BACKGROUND is not possible at all
    // Keyboard doesn't use DISCL_EXCLUSIVE at all, because then the WM_CHAR and
    // WM_KEYDOWN wouldn't be processed
    if (InputDevices.DI)
        if (OK(InputDevices.DI->CreateDevice(GUID_SysKeyboard, &_device, null))) {
            if (OK(_device->SetDataFormat(&c_dfDIKeyboard)))
                if (OK(_device->SetCooperativeLevel(
                        App.window(),
                        (_exclusive ? DISCL_NOWINKEY : 0) | DISCL_NONEXCLUSIVE |
                            ((KEYBOARD_MODE == FOREGROUND) ? DISCL_FOREGROUND
                                                           : DISCL_BACKGROUND)))) {
                    DIPROPDWORD dipdw;
                    dipdw.diph.dwSize = SIZE(DIPROPDWORD);
                    dipdw.diph.dwHeaderSize = SIZE(DIPROPHEADER);
                    dipdw.diph.dwObj = 0;
                    dipdw.diph.dwHow = DIPH_DEVICE;
                    dipdw.dwData = BUF_KEYS;
                    _device->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

                    if (KEYBOARD_MODE == BACKGROUND)
                        _device->Acquire(); // in background mode we always want the
                                            // keyboard to be acquired
                    _hardware = true;
                    goto ok;
                }
            RELEASE(_device);
        }
ok:;
#endif
#elif WINDOWS_NEW
        // detection is done in 'checkMouseKeyboard'
#elif MAC || LINUX || WEB
    _hardware = true; // FIXME: TODO:
#endif
}
/******************************************************************************
static Byte ShiftMap[3][128];
static void Init()
{
   REPD(shift, 2)
   REPD(caps , 2)if(shift || caps)
   {
      Int m=((shift | (caps<<1))-1);
      REPA(ShiftMap[m])ShiftMap[m][i]=((shift==caps) ? Char8(i) :
CaseUp(Char8(i))); if(shift)
      {
         ShiftMap[m][Unsigned('`')]='~';
         ShiftMap[m][Unsigned('1')]='!';
         ShiftMap[m][Unsigned('2')]='@';
         ShiftMap[m][Unsigned('3')]='#';
         ShiftMap[m][Unsigned('4')]='$';
         ShiftMap[m][Unsigned('5')]='%';
         ShiftMap[m][Unsigned('6')]='^';
         ShiftMap[m][Unsigned('7')]='&';
         ShiftMap[m][Unsigned('8')]='*';
         ShiftMap[m][Unsigned('9')]='(';
         ShiftMap[m][Unsigned('0')]=')';
         ShiftMap[m][Unsigned('-')]='_';
         ShiftMap[m][Unsigned('=')]='+';
         ShiftMap[m][Unsigned('[')]='{';
         ShiftMap[m][Unsigned(']')]='}';
         ShiftMap[m][Unsigned(';')]=':';
         ShiftMap[m][Unsigned('\'')]='"';
         ShiftMap[m][Unsigned('\\')]='|';
         ShiftMap[m][Unsigned(',')]='<';
         ShiftMap[m][Unsigned('.')]='>';
         ShiftMap[m][Unsigned('/')]='?';
      }
   }
}
static Char AdjustByShift(Char c, Bool shift, Bool caps)
{
   if(shift || caps)
   {
      if(InRange(Unsigned(c), ShiftMap[0]))return ShiftMap[(shift |
(caps<<1))-1][Unsigned(c)]; if(shift!=caps)return CaseUp(c);
   }
   return c;
}
/******************************************************************************/
Char KeyboardClass::keyChar(KB_KEY k) C {
    ASSERT(1 << (8 * SIZE(k)) == ELMS(_key_char));
    return _key_char[k];
}
CChar8 *KeyboardClass::keyName(KB_KEY k) C {
    ASSERT(1 << (8 * SIZE(k)) == ELMS(_key_name));
    return _key_name[k];
}
CChar *KeyboardClass::keySymbol(KB_KEY k) C {
    ASSERT(1 << (8 * SIZE(k)) == ELMS(_key_symbol));
    return _key_symbol[k];
}
#if ANDROID || SWITCH
Char KeyboardClass::keyChar(KB_KEY k, Bool shift, Bool caps) C {
    if (k >= KB_A && k <= KB_Z)
        shift ^= caps; // caps only affects A..Z
    return shift ? _key_char_shift[k] : _key_char[k];
}
#endif
/******************************************************************************/
#if WINDOWS_OLD
#define KB_F13 KB_NONE
#define KB_F14 KB_NONE
#define KB_F15 KB_NONE
#define KB_F16 KB_NONE
#define KB_F17 KB_NONE
#define KB_F18 KB_NONE
#define KB_F19 KB_NONE
#if 1 // these keys should be ignored because they're different even though US  \
      // keyboard was selected, so when US keyboard is selected, then '_qwerty' \
      // and 'MapVirtualKey' have 1:1 mapping
#define KB_PRINT KB_NONE
#define KB_NP5 KB_NONE
#define KB_PAUSE KB_NONE
#define KB_LWIN KB_NONE
#define KB_RWIN KB_NONE
#define KB_MENU KB_NONE
#endif
static const KB_KEY ScanCodeToQwertyKey[] = {
    //    0        1        2        3          4        5         6          7
    //    8        9         A          B          C         D         E F
    KB_NONE, KB_ESC, KB_1, KB_2, KB_3, KB_4,
    KB_5, KB_6, KB_7, KB_8, KB_9, KB_0,
    KB_SUB, KB_EQUAL, KB_BACK, KB_TAB, // 0
    KB_Q, KB_W, KB_E, KB_R, KB_T, KB_Y,
    KB_U, KB_I, KB_O, KB_P, KB_LBR, KB_RBR,
    KB_ENTER, KB_LCTRL, KB_A, KB_S, // 1
    KB_D, KB_F, KB_G, KB_H, KB_J, KB_K,
    KB_L, KB_SEMI, KB_APO, KB_TILDE, KB_LSHIFT, KB_BSLASH,
    KB_Z, KB_X, KB_C, KB_V, // 2
    KB_B, KB_N, KB_M, KB_COMMA, KB_DOT, KB_SLASH,
    KB_RSHIFT, KB_PRINT, KB_LALT, KB_SPACE, KB_CAPS, KB_F1,
    KB_F2, KB_F3, KB_F4, KB_F5, // 3
    KB_F6, KB_F7, KB_F8, KB_F9, KB_F10, KB_NUM,
    KB_SCROLL, KB_HOME, KB_UP, KB_PGUP, KB_NPSUB, KB_LEFT,
    KB_NP5, KB_RIGHT, KB_NPADD, KB_END, // 4
    KB_DOWN, KB_PGDN, KB_INS, KB_DEL, KB_NONE, KB_NONE,
    KB_NONE, KB_F11, KB_F12, KB_PAUSE, KB_NONE, KB_LWIN,
    KB_RWIN, KB_MENU, KB_NONE, KB_NONE, // 5
    KB_NONE, KB_NONE, KB_NONE, KB_NONE, KB_F13, KB_F14,
    KB_F15, KB_F16, KB_F17, KB_F18, KB_F19, KB_NONE,
    KB_NONE, KB_NONE, KB_NONE, KB_NONE, // 6
    KB_NONE, KB_NONE, KB_NONE, KB_NONE, KB_NONE, KB_NONE,
    KB_NONE, KB_NONE, KB_NONE, KB_NONE, KB_NONE, KB_NONE,
    KB_NONE, KB_NONE, KB_NONE, KB_NONE, // 7
};
#undef KB_PRINT
#undef KB_NP5
#undef KB_PAUSE
#undef KB_LWIN
#undef KB_RWIN
#undef KB_MENU
#elif MAC
KB_KEY ScanCodeToKey[Elms(ScanCodeToQwertyKey)];
const KB_KEY ScanCodeToQwertyKey[0x80] = {
    // 0x00
    KB_A,
    KB_S,
    KB_D,
    KB_F,
    KB_H,
    KB_G,
    KB_Z,
    KB_X,
    KB_C,
    KB_V,
    KB_NONE,
    KB_B,
    KB_Q,
    KB_W,
    KB_E,
    KB_R,
    // 0x10
    KB_Y,
    KB_T,
    KB_1,
    KB_2,
    KB_3,
    KB_4,
    KB_6,
    KB_5,
    KB_EQUAL,
    KB_9,
    KB_7,
    KB_SUB,
    KB_8,
    KB_0,
    KB_RBR,
    KB_O,
    // 0x20
    KB_U,
    KB_LBR,
    KB_I,
    KB_P,
    KB_ENTER,
    KB_L,
    KB_J,
    KB_APO,
    KB_K,
    KB_SEMI,
    KB_BSLASH,
    KB_COMMA,
    KB_SLASH,
    KB_N,
    KB_M,
    KB_DOT,
    // 0x30
    KB_TAB,
    KB_SPACE,
    KB_TILDE,
    KB_BACK,
    KB_NONE, // KB_NP_ENTER
    KB_ESC,
    KB_NONE, // KB_RWIN
    KB_NONE, // KB_LWIN
    KB_NONE, // KB_LSHIFT
    KB_NONE, // KB_CAPS
    KB_NONE, // KB_LALT
    KB_NONE, // KB_LCTRL
    KB_NONE, // KB_RSHIFT
    KB_NONE, // KB_RALT
    KB_NONE, // KB_RCTRL
    KB_NONE,
    // 0x40
    KB_NONE,
    KB_NPDEL,
    KB_NONE,
    KB_NPMUL,
    KB_NONE,
    KB_NPADD,
    KB_NONE,
    KB_NUM,
    KB_VOL_UP,
    KB_VOL_DOWN,
    KB_VOL_MUTE,
    KB_NPDIV,
    KB_NPENTER,
    KB_NONE,
    KB_NPSUB,
    KB_NONE,
    // 0x50
    KB_NONE,
    KB_NONE,
    KB_NP0,
    KB_NP1,
    KB_NP2,
    KB_NP3,
    KB_NP4,
    KB_NP5,
    KB_NP6,
    KB_NP7,
    KB_NONE,
    KB_NP8,
    KB_NP9,
    KB_NONE,
    KB_NONE,
    KB_NONE,
    // 0x60
    KB_F5,
    KB_F6,
    KB_F7,
    KB_F3,
    KB_F8,
    KB_F9,
    KB_NONE,
    KB_F11,
    KB_NONE,
    KB_PRINT,
    KB_NONE,
    KB_SCROLL,
    KB_NONE,
    KB_F10,
    KB_MENU,
    KB_F12,
    // 0x70
    KB_NONE,
    KB_PAUSE,
    KB_INS,
    KB_HOME,
    KB_PGUP,
    KB_DEL,
    KB_F4,
    KB_END,
    KB_F2,
    KB_PGDN,
    KB_F1,
    KB_LEFT,
    KB_RIGHT,
    KB_DOWN,
    KB_UP,
};
#elif LINUX
KB_KEY ScanCodeToKey[Elms(ScanCodeToQwertyKey)];
const KB_KEY ScanCodeToQwertyKey[0x90] = {
    KB_NONE, // 0x00
    KB_NONE, // 0x01
    KB_NONE, // 0x02
    KB_NONE, // 0x03
    KB_NONE, // 0x04
    KB_NONE, // 0x05
    KB_NONE, // 0x06
    KB_NONE, // 0x07
    KB_NONE, // 0x08
    KB_ESC,  // 0x09
    KB_1,    // 0x0A
    KB_2,    // 0x0B
    KB_3,    // 0x0C
    KB_4,    // 0x0D
    KB_5,    // 0x0E
    KB_6,    // 0x0F

    KB_7,     // 0x10
    KB_8,     // 0x11
    KB_9,     // 0x12
    KB_0,     // 0x13
    KB_SUB,   // 0x14
    KB_EQUAL, // 0x15
    KB_BACK,  // 0x16
    KB_TAB,   // 0x17
    KB_Q,     // 0x18
    KB_W,     // 0x19
    KB_E,     // 0x1A
    KB_R,     // 0x1B
    KB_T,     // 0x1C
    KB_Y,     // 0x1D
    KB_U,     // 0x1E
    KB_I,     // 0x1F

    KB_O,     // 0x20
    KB_P,     // 0x21
    KB_LBR,   // 0x22
    KB_RBR,   // 0x23
    KB_ENTER, // 0x24
    KB_LCTRL, // 0x25
    KB_A,     // 0x26
    KB_S,     // 0x27
    KB_D,     // 0x28
    KB_F,     // 0x29
    KB_G,     // 0x2A
    KB_H,     // 0x2B
    KB_J,     // 0x2C
    KB_K,     // 0x2D
    KB_L,     // 0x2E
    KB_SEMI,  // 0x2F

    KB_APO,       // 0x30
    KB_TILDE,     // 0x31
    KB_LSHIFT,    // 0x32
    KB_BACKSLASH, // 0x33
    KB_Z,         // 0x34
    KB_X,         // 0x35
    KB_C,         // 0x36
    KB_V,         // 0x37
    KB_B,         // 0x38
    KB_N,         // 0x39
    KB_M,         // 0x3A
    KB_COMMA,     // 0x3B
    KB_DOT,       // 0x3C
    KB_SLASH,     // 0x3D
    KB_RSHIFT,    // 0x3E
    KB_NPMUL,     // 0x3F

    KB_LALT,   // 0x40
    KB_SPACE,  // 0x41
    KB_NONE,   // 0x42
    KB_F1,     // 0x43
    KB_F2,     // 0x44
    KB_F3,     // 0x45
    KB_F4,     // 0x46
    KB_F5,     // 0x47
    KB_F6,     // 0x48
    KB_F7,     // 0x49
    KB_F8,     // 0x4A
    KB_F9,     // 0x4B
    KB_F10,    // 0x4C
    KB_NUM,    // 0x4D
    KB_SCROLL, // 0x4E
    KB_NP7,    // 0x4F

    KB_NP8,   // 0x50
    KB_NP9,   // 0x51
    KB_NPSUB, // 0x52
    KB_NP4,   // 0x53
    KB_NP5,   // 0x54
    KB_NP6,   // 0x55
    KB_NPADD, // 0x56
    KB_NP1,   // 0x57
    KB_NP2,   // 0x58
    KB_NP3,   // 0x59
    KB_NP0,   // 0x5A
    KB_NPDEL, // 0x5B
    KB_NONE,  // 0x5C
    KB_NONE,  // 0x5D
    KB_NONE,  // 0x5E
    KB_F11,   // 0x5F

    KB_F12,     // 0x60
    KB_NONE,    // 0x61
    KB_NONE,    // 0x62
    KB_NONE,    // 0x63
    KB_NONE,    // 0x64
    KB_NONE,    // 0x65
    KB_NONE,    // 0x66
    KB_NONE,    // 0x67
    KB_NPENTER, // 0x68
    KB_RCTRL,   // 0x69
    KB_NPDIV,   // 0x6A
    KB_NONE,    // 0x6B
    KB_RALT,    // 0x6C
    KB_NONE,    // 0x6D
    KB_HOME,    // 0x6E
    KB_UP,      // 0x6F

    KB_PGUP,  // 0x70
    KB_LEFT,  // 0x71
    KB_RIGHT, // 0x72
    KB_END,   // 0x73
    KB_DOWN,  // 0x74
    KB_PGDN,  // 0x75
    KB_INS,   // 0x76
    KB_DEL,   // 0x77
    KB_NONE,  // 0x78
    KB_NONE,  // 0x79
    KB_NONE,  // 0x7A
    KB_NONE,  // 0x7B
    KB_NONE,  // 0x7C
    KB_NONE,  // 0x7D
    KB_NONE,  // 0x7E
    KB_PAUSE, // 0x7F

    KB_NONE, // 0x80
    KB_NONE, // 0x81
    KB_NONE, // 0x82
    KB_NONE, // 0x83
    KB_NONE, // 0x84
    KB_LWIN, // 0x85
    KB_RWIN, // 0x86
    KB_MENU, // 0x87
    KB_NONE, // 0x88
    KB_NONE, // 0x89
    KB_NONE, // 0x8A
    KB_NONE, // 0x8B
    KB_NONE, // 0x8C
    KB_NONE, // 0x8D
    KB_NONE, // 0x8E
    KB_NONE, // 0x8F
};
#endif
#if MAC || LINUX
static const KB_KEY VariableKeys[] = // keys that may change depending on layout
    {
        KB_OEM_102, // !! put this first on the list so it's processed last in
                    // REPA loop (to avoid selecting it unless necessary,
                    // because it's uncommon key) !!

        KB_TILDE,
        KB_0,
        KB_1,
        KB_2,
        KB_3,
        KB_4,
        KB_5,
        KB_6,
        KB_7,
        KB_8,
        KB_9,
        KB_SUB,
        KB_EQUAL,
        KB_Q,
        KB_W,
        KB_E,
        KB_R,
        KB_T,
        KB_Y,
        KB_U,
        KB_I,
        KB_O,
        KB_P,
        KB_LBR,
        KB_RBR,
        KB_BSLASH,
        KB_A,
        KB_S,
        KB_D,
        KB_F,
        KB_G,
        KB_H,
        KB_J,
        KB_K,
        KB_L,
        KB_SEMI,
        KB_APO,
        KB_Z,
        KB_X,
        KB_C,
        KB_V,
        KB_B,
        KB_N,
        KB_M,
        KB_COMMA,
        KB_DOT,
        KB_SLASH,
};
#endif
struct KeyData {
    Byte scan_code, cs;
    Char c[3];

    void add(Char c) {
        T.c[cs++] = CaseUp(c);
    } // add CaseUp so later when checking for characters we can just do >='A' &&
      // <='Z', use Up instead of Down because KB_A is 'A'
};
void KeyboardClass::setLayout() {
    REPAO(_qwerty) = KB_KEY(i);
#if WINDOWS
#if 1 && WINDOWS_OLD // this is better, but 'MapVirtualKey' is available only on \
                     // WINDOWS_OLD
    FREPA(ScanCodeToQwertyKey)
    if (KB_KEY k = ScanCodeToQwertyKey[i])
        if (UInt vk = MapVirtualKey(
                i,
                MAPVK_VSC_TO_VK_EX))
            if (k !=
                vk)
                _qwerty[k] = KB_KEY(vk);
#else
#if WINDOWS_OLD
    LANG_TYPE lang = LANG_TYPE((UInt(UIntPtr(GetKeyboardLayout(0))) >> 16) &
                               0xFF); // LOWORD represents the language, while
                                      // HIWORD represents the keyboard layout
#elif WINDOWS_NEW
    LANG_TYPE lang = LanguageCode(
        Windows::Globalization::Language::CurrentInputMethodLanguageTag->Data());
#endif
    switch (lang) {
    // conversion map was detected by changing to another keyboard layout in the
    // OS, and observing what was received in WM_KEYDOWN while pressing keys
    case LANG_SLOVENIAN: // QWERTZ
    case LANG_CROATIAN:  // LANG_BOSNIAN, LANG_SERBIAN - QWERTZ
    {
        _qwerty[KB_Y] = KB_Z;
        _qwerty[KB_Z] = KB_Y;
        _qwerty[KB_SUB] = KB_SLASH;
        _qwerty[KB_SLASH] = KB_SUB;
    } break;

    case LANG_CZECH: // QWERTZ
    {
        _qwerty[KB_Y] = KB_Z;
        _qwerty[KB_Z] = KB_Y;
        _qwerty[KB_SUB] = KB_EQUAL;
        _qwerty[KB_EQUAL] = KB_SLASH;
        _qwerty[KB_SLASH] = KB_SUB;
    } break;

    case LANG_DUTCH: // AZERTY
    {
        _qwerty[KB_Q] = KB_A;
        _qwerty[KB_A] = KB_Q;
        _qwerty[KB_W] = KB_Z;
        _qwerty[KB_Z] = KB_W;
        _qwerty[KB_SUB] = KB_LBR;
        _qwerty[KB_EQUAL] = KB_SUB;
        _qwerty[KB_LBR] = KB_RBR;
        _qwerty[KB_RBR] = KB_SEMI;
        _qwerty[KB_SEMI] = KB_M;
        _qwerty[KB_APO] = KB_TILDE;
        _qwerty[KB_TILDE] = KB_APO;
        _qwerty[KB_M] = KB_COMMA;
        _qwerty[KB_COMMA] = KB_DOT;
        _qwerty[KB_DOT] = KB_SLASH;
        _qwerty[KB_SLASH] = KB_EQUAL;
    } break;

    case LANG_ESTONIAN: // QWERTY
    {
        _qwerty[KB_TILDE] = KB_APO;
        _qwerty[KB_LBR] = KB_TILDE;
        _qwerty[KB_RBR] = KB_LBR;
        _qwerty[KB_APO] = KB_SLASH;
        _qwerty[KB_SLASH] = KB_RBR;
    } break;

    case LANG_FINNISH:   // QWERTY
    case LANG_NORWEGIAN: // QWERTY
    case LANG_DANISH:    // QWERTY
    case LANG_SWEDISH:   // QWERTY
    {
        _qwerty[KB_TILDE] = KB_BSLASH;
        _qwerty[KB_SUB] = KB_EQUAL;
        _qwerty[KB_EQUAL] = KB_LBR;
        _qwerty[KB_LBR] = KB_RBR;
        _qwerty[KB_RBR] = KB_SEMI;
        _qwerty[KB_BSLASH] = KB_SLASH;
        _qwerty[KB_SEMI] = KB_TILDE;
        _qwerty[KB_SLASH] = KB_SUB;
    } break;

    case LANG_FRENCH: // AZERTY
    {
        _qwerty[KB_Q] = KB_A;
        _qwerty[KB_A] = KB_Q;
        _qwerty[KB_W] = KB_Z;
        _qwerty[KB_Z] = KB_W;
        _qwerty[KB_SUB] = KB_LBR;
        _qwerty[KB_LBR] = KB_RBR;
        _qwerty[KB_RBR] = KB_SEMI;
        _qwerty[KB_SEMI] = KB_M;
        _qwerty[KB_APO] = KB_TILDE;
        _qwerty[KB_TILDE] = KB_APO;
        _qwerty[KB_M] = KB_COMMA;
        _qwerty[KB_COMMA] = KB_DOT;
        _qwerty[KB_DOT] = KB_SLASH;
        _qwerty[KB_SLASH] = KB_KEY(223);
        _qwerty[223] =
            KB_NONE; // VK_OEM_8 was received in WM_KEYDOWN when pressing '/', need
                     // to disable _qwerty[VK_OEM_8] so reverse convert will work
    } break;

    case LANG_GERMAN: // QWERTZ
    {
        _qwerty[KB_Y] = KB_Z;
        _qwerty[KB_Z] = KB_Y;
        _qwerty[KB_SUB] = KB_LBR;
        _qwerty[KB_EQUAL] = KB_RBR;
        _qwerty[KB_LBR] = KB_SEMI;
        _qwerty[KB_RBR] = KB_EQUAL;
        _qwerty[KB_TILDE] = KB_BACKSLASH;
        _qwerty[KB_BACKSLASH] = KB_SLASH;
        _qwerty[KB_SEMI] = KB_TILDE;
        _qwerty[KB_SLASH] = KB_SUB;
    } break;

    case LANG_HUNGARIAN: // QWERTZ
    {
        _qwerty[KB_Y] = KB_Z;
        _qwerty[KB_Z] = KB_Y;
        _qwerty[KB_TILDE] = KB_0;
        _qwerty[KB_0] = KB_TILDE;
        _qwerty[KB_SUB] = KB_SLASH;
        _qwerty[KB_SLASH] = KB_SUB;
    } break;

    case LANG_SPANISH: // QWERTY
    case LANG_ITALIAN: // QWERTY
    {
        _qwerty[KB_TILDE] = KB_BSLASH;
        _qwerty[KB_SUB] = KB_LBR;
        _qwerty[KB_EQUAL] = KB_RBR;
        _qwerty[KB_LBR] = KB_SEMI;
        _qwerty[KB_RBR] = KB_EQUAL;
        _qwerty[KB_BSLASH] = KB_SLASH;
        _qwerty[KB_SEMI] = KB_TILDE;
        _qwerty[KB_SLASH] = KB_SUB;
    } break;

    case LANG_LUXEMBOURGISH: // QWERTZ
    {
        _qwerty[KB_Y] = KB_Z;
        _qwerty[KB_Z] = KB_Y;
        _qwerty[KB_TILDE] = KB_SLASH;
        _qwerty[KB_SUB] = KB_LBR;
        _qwerty[KB_EQUAL] = KB_RBR;
        _qwerty[KB_LBR] = KB_SEMI;
        _qwerty[KB_RBR] = KB_TILDE;
        _qwerty[KB_SEMI] = KB_APO;
        _qwerty[KB_APO] = KB_BSLASH;
        _qwerty[KB_SLASH] = KB_SUB;
    } break;

    case LANG_PORTUGUESE: // QWERTY
    {
        _qwerty[KB_TILDE] = KB_BSLASH;
        _qwerty[KB_SUB] = KB_LBR;
        _qwerty[KB_EQUAL] = KB_RBR;
        _qwerty[KB_LBR] = KB_EQUAL;
        _qwerty[KB_RBR] = KB_SEMI;
        _qwerty[KB_BSLASH] = KB_SLASH;
        _qwerty[KB_SEMI] = KB_TILDE;
        _qwerty[KB_SLASH] = KB_SUB;
    } break;

    case LANG_SLOVAK: // QWERTZ
    {
        _qwerty[KB_Y] = KB_Z;
        _qwerty[KB_Z] = KB_Y;
        _qwerty[KB_SUB] = KB_SLASH;
        _qwerty[KB_EQUAL] = KB_KEY(223);
        _qwerty[223] =
            KB_NONE; // VK_OEM_8 was received in WM_KEYDOWN when pressing '-', need
                     // to disable _qwerty[VK_OEM_8] so reverse convert will work
        _qwerty[KB_SLASH] = KB_SUB;
    } break;
    case LANG_TURKISH: // QWERTY
    {
        _qwerty[KB_SUB] = KB_KEY(223);
        _qwerty[223] =
            KB_NONE; // VK_OEM_8 was received in WM_KEYDOWN when pressing '-', need
                     // to disable _qwerty[VK_OEM_8] so reverse convert will work
        _qwerty[KB_EQUAL] = KB_SUB;
        _qwerty[KB_BSLASH] = KB_COMMA;
        _qwerty[KB_COMMA] = KB_SLASH;
        _qwerty[KB_DOT] = KB_BSLASH;
        _qwerty[KB_SLASH] = KB_DOT;
    } break;
    }
#endif
#elif MAC || LINUX
    Bool ok = false;

#if MAC
    if (TISInputSourceRef key_layout =
            TISCopyCurrentKeyboardLayoutInputSource()) {
        if (CFDataRef unicode_data = (CFDataRef)TISGetInputSourceProperty(
                key_layout, kTISPropertyUnicodeKeyLayoutData))
            if (UCKeyboardLayout *data =
                    (UCKeyboardLayout *)CFDataGetBytePtr(unicode_data)) {
                UInt keyboard_type = LMGetKbdType();
#elif LINUX
    if (XDisplay) {
#endif

                ok = true;
                Bool variable_key[256];
                Zero(variable_key);
                REPA(VariableKeys)
                variable_key[VariableKeys[i]] = true;
                MemtN<KeyData, Elms(VariableKeys)> kds;
                FREPA(ScanCodeToKey) {
                    KB_KEY qwerty_key = ScanCodeToQwertyKey[i];
                    if (variable_key
                            [qwerty_key]) // if this is a variable key (can change), then
                                          // get what pressing it will give us (what
                                          // characters), remember up to 3 combinations,
                                          // default, with shift, and with right alt
                    {
                        KeyData kd;
                        kd.cs = 0;

#if MAC
                        UniCharCount len;
                        UniChar s[8];
                        UInt dead_key_state = 0;
                        if (UCKeyTranslate(data, i, kUCKeyActionDown, 0x00, keyboard_type,
                                           kUCKeyTranslateNoDeadKeysMask, &dead_key_state,
                                           Elms(s), &len, s) == noErr)
                            if (len > 0)
                                kd.add(s[0]);
                        dead_key_state = 0;
                        if (UCKeyTranslate(data, i, kUCKeyActionDown, 0x02, keyboard_type,
                                           kUCKeyTranslateNoDeadKeysMask, &dead_key_state,
                                           Elms(s), &len, s) == noErr)
                            if (len > 0)
                                kd.add(s[0]); // with Shift
                        dead_key_state = 0;
                        if (UCKeyTranslate(data, i, kUCKeyActionDown, 0x40, keyboard_type,
                                           kUCKeyTranslateNoDeadKeysMask, &dead_key_state,
                                           Elms(s), &len, s) == noErr)
                            if (len > 0)
                                kd.add(s[0]); // with RightAlt
#elif LINUX
                KeySym ks = XkbKeycodeToKeysym(XDisplay, KeyCode(i), 0, 0);
                if (ks != NoSymbol)
                    kd.add(Char(ks));
                ks = XkbKeycodeToKeysym(XDisplay, KeyCode(i), 0, 1);
                if (ks != NoSymbol)
                    kd.add(Char(ks)); // Shift
#endif

                        if (kd.cs) {
                            kd.scan_code = i;
                            kds.New() = kd;
                        } // if has any character mapping, then store into list to process
                          // later

                        ScanCodeToKey[i] = KB_NONE; // set as unassigned
                    } else
                        ScanCodeToKey[i] = qwerty_key; // assign keys that are not variable
                }
                Bool(&unassigned_key)[256] =
                    variable_key; // we can reuse this array, because only variable keys
                                  // are unassigned
                // assign characters first, priority #1
                REPAD(ki, kds) {
                    C KeyData &kd = kds[ki];
                    REPD(ci, kd.cs) {
                        Char c = kd.c[ci];
                        ASSERT(KB_A == 'A' && KB_Z == 'Z');
                        if (c >= 'A' && c <= 'Z')
                            if (unassigned_key[KB_KEY(c)]) {
                                unassigned_key[KB_KEY(c)] = false;
                                ScanCodeToKey[kd.scan_code] = KB_KEY(c);
                                kds.remove(ki);
                                goto next_chr;
                            }
                    }
                next_chr:;
                }
                // assign digits, priority #2
                REPAD(ki, kds) {
                    C KeyData &kd = kds[ki];
                    REPD(ci, kd.cs) {
                        Char c = kd.c[ci];
                        ASSERT(KB_0 == '0' && KB_9 == '9');
                        if (c >= '0' && c <= '9')
                            if (unassigned_key[KB_KEY(c)]) {
                                unassigned_key[KB_KEY(c)] = false;
                                ScanCodeToKey[kd.scan_code] = KB_KEY(c);
                                kds.remove(ki);
                                goto next_dig;
                            }
                    }
                next_dig:;
                }
                // assign symbols, priority #3
                REPAD(ki, kds) {
                    C KeyData &kd = kds[ki];
                    REPD(ci, kd.cs) {
                        Char c = kd.c[ci];
                        if (!(c >= 'A' && c <= 'Z') && !(c >= '0' && c <= '9'))
                            REPA(VariableKeys) {
                                KB_KEY k = VariableKeys[i];
                                if (Kb._key_char[k] == c) {
                                    if (unassigned_key[k]) {
                                        unassigned_key[k] = false;
                                        ScanCodeToKey[kd.scan_code] = k;
                                        kds.remove(ki);
                                        goto next_sym;
                                    }
                                    break;
                                }
                            }
                    }
                next_sym:;
                }
                // assign with QWERTY mapping, priority #4
                REPAD(ki, kds) {
                    C KeyData &kd = kds[ki];
                    KB_KEY k = ScanCodeToQwertyKey[kd.scan_code];
                    if (unassigned_key[k]) {
                        unassigned_key[k] = false;
                        ScanCodeToKey[kd.scan_code] = k;
                        kds.remove(ki);
                    }
                }
                // assign remaining to any key that was not assigned yet
                REPAD(ki, kds) {
                    C KeyData &kd = kds[ki];
                    REPA(VariableKeys) // !! here use REPA to go from the back, to process
                                       // KB_OEM_102 last (to avoid selecting it unless
                                       // necessary, because it's uncommon key) !!
                    {
                        KB_KEY k = VariableKeys[i];
                        if (unassigned_key[k]) {
                            unassigned_key[k] = false;
                            ScanCodeToKey[kd.scan_code] = k;
                            break;
                        }
                    }
                }

#if MAC
            }
        CFRelease(key_layout);
    }
#elif LINUX
    }
#endif

    if (!ok)
        Copy(ScanCodeToKey, ScanCodeToQwertyKey); // if couldn't obtain keyboard
                                                  // layout, then set as QWERTY
    REPAO(_qwerty) = KB_KEY(i);
    REPA(ScanCodeToKey)
    if (KB_KEY qwerty_key = ScanCodeToQwertyKey[i])
        if (KB_KEY key =
                ScanCodeToKey[i])
            _qwerty[qwerty_key] = key;
#endif
}
void KeyboardClass::swapCtrlCmd(Bool swapped) { T._swap_ctrl_cmd = swapped; }
/******************************************************************************/
#if WINDOWS_OLD
Bool KeyboardClass::imm() C {
    HIMC imc = ImmGetContext(App.window());
    if (imc)
        ImmReleaseContext(App.window(), imc);
    return imc != null;
}
void KeyboardClass::imm(Bool enable) {
    if (_imm != enable) {
        _imm = enable;
        ImmAssociateContext(App.window(), enable ? _imc : null);
    }
}
Bool KeyboardClass::immNative() C { return ImmGetOpenStatus(_imc) != 0; }
void KeyboardClass::immNative(Bool native) {
    ImmSetOpenStatus(_imc, native);
    if (native) {
        LANG_TYPE lang = LANG_TYPE((UInt(UIntPtr(GetKeyboardLayout(0))) >> 16) &
                                   0xFF); // LOWORD represents the language, while
                                          // HIWORD represents the keyboard layout
        if (lang == LANG_KOREAN) {
            DWORD c, s;
            ImmGetConversionStatus(_imc, &c, &s);
            ImmSetConversionStatus(_imc, c | 1, s);
        }
    }
}
#else
Bool KeyboardClass::immNative() C { return false; }
void KeyboardClass::immNative(Bool native) {}
#endif
/******************************************************************************/
void KeyboardClass::clear() {
    k.clear();
    _last_key_scan_code = -1;
    REPAO(_button) &= ~BS_NOT_ON;
}
/******************************************************************************/
INLINE static void AddModifiers(KeyboardKey &k) {
    if (Kb.anyCtrl())
        k.flags |= KeyboardKey::CTRL;
    if (Kb.anyShift())
        k.flags |= KeyboardKey::SHIFT;
    if (Kb.anyAlt())
        k.flags |= KeyboardKey::ALT;
    if (Kb.anyWin())
        k.flags |= KeyboardKey::WIN;
    if (Kb.b(KB_LALT))
        k.flags |= KeyboardKey::LALT;
}
#define QUEUEABLE_CHR 32
void KeyboardClass::queue(Char chr, Int scan_code) {
    if (Unsigned(chr) >= QUEUEABLE_CHR) {
        if (_last_key_scan_code == scan_code && scan_code >= 0 && _key_buffer_len) {
            KeyboardKey &last_key =
                _key_buffer[(_key_buffer_pos + _key_buffer_len - 1) & 0xFF];
            if (!last_key.c) {
                last_key.c = chr;
                return;
            }
        }
        KeyboardKey k;
        k.c = chr;
        k.k = KB_NONE;
        k.flags = KeyboardKey::FIRST;
        AddModifiers(k);
        queue(k);
        _last_key_scan_code = scan_code;
    }
}
void KeyboardClass::push(KB_KEY key, Int scan_code) {
    if (key) {
        KeyboardKey k;
        if (_button[key] & BS_ON) // repeated press
        {
            k.flags = 0;
            _button[key] |= BS_REPEAT;
        } else // first press
        {
            k.flags = KeyboardKey::FIRST;
            _cur = key; // remember most recent key pressed, to prevent cursor from
                        // blinking while holding that key
            _button[key] |= BS_PUSHED | BS_ON;
            if (_last == key &&
                Time.appTime() - _last_t <= DoubleClickTime + Time.ad()) {
                _button[key] |= BS_DOUBLE;
                _last = -1;
            } else {
                _last = key;
                _last_t = Time.appTime();
            }
            Inputs.New().set(true, INPUT_KEYBOARD, key);
        }
        // !! set modifier flags after adjusting '_button' above !!
        k.c = '\0';
        k.k = key;
        AddModifiers(k);
        queue(k);
        _last_key_scan_code = scan_code;
    }
}
void KeyboardClass::release(KB_KEY key) {
    if (_button[key] & BS_ON) {
        if (_cur == key)
            _cur = -1;
        FlagDisable(_button[key], BS_ON);
        FlagEnable(_button[key], BS_RELEASED);
        Inputs.New().set(false, INPUT_KEYBOARD, key);
    }
}
void KeyboardClass::queue(KB_KEY key, Char chr) {
    if (Unsigned(chr) < QUEUEABLE_CHR)
        chr = '\0';
    if (key || chr) {
        KeyboardKey k;
        k.flags = KeyboardKey::FIRST;
        k.c = chr;
        k.k = key;
        queue(k);
    }
}
/*void KeyboardClass::tap(KB_KEY key, Char chr) // push(key), queue(chr),
release(key)
{
   if(Unsigned(chr)<QUEUEABLE_CHR)chr='\0';
   if(key || chr)
   {
      KeyboardKey k;
      if(_button[key]&BS_ON) // repeated press
      {
         k.flags=0;
        _button[key]|=BS_REPEAT;
      }else // first press
      {
         k.flags=KeyboardKey::FIRST;
        _button[key]|=BS_PUSHED|BS_RELEASED;
         if(_last==key && Time.appTime()-_last_t<=DoubleClickTime+Time.ad())
         {
           _button[key]|=BS_DOUBLE;
           _last        =-1;
         }else
         {
           _last  =key;
           _last_t=Time.appTime();
         }
         InputCombo.add(InputButton(INPUT_KEYBOARD, key));
      }
      k.c=chr;
      k.k=key;
      queue(k);
   }
}
/******************************************************************************/
void KeyboardClass::setModifiers() {
    _ctrl = FlagOn(_button[KB_LCTRL] | _button[KB_RCTRL], BS_ON | BS_PUSHED);
    _shift = FlagOn(_button[KB_LSHIFT] | _button[KB_RSHIFT], BS_ON | BS_PUSHED);
    _alt = FlagOn(_button[KB_LALT] | _button[KB_RALT], BS_ON | BS_PUSHED);
    _win = FlagOn(_button[KB_LWIN] | _button[KB_RWIN], BS_ON | BS_PUSHED);
}
void KeyboardClass::update() {
#if WINDOWS_OLD
#if KB_DIRECT_INPUT
    if (App.active() && _device) {
        DIDEVICEOBJECTDATA didod[BUF_KEYS];
        DWORD elms = BUF_KEYS,
              ret =
                  _device->GetDeviceData(SIZE(DIDEVICEOBJECTDATA), didod, &elms, 0);
        if (ret == DI_OK || ret == DI_BUFFEROVERFLOW)
            FREP(elms) // process in order
            {
                C DIDEVICEOBJECTDATA &d = didod[i];
                Byte dik = d.dwOfs;
                REPA(Keys) // process only special keys
                {
                    C DIK &key = Keys[i];
                    if (key.dik == dik) {
                        if (d.dwData & 0x80)
                            push(key.key, key.scan_code);
                        else
                            release(key.key);
                        break;
                    }
                }
            }
        if (ret != DI_OK) // if we failed to catch entire input, then check most
                          // recent state
        {
            Byte dik[256];
            if (!OK(_device->GetDeviceState(SIZE(dik), &dik))) // if failed
            {
                _device->Acquire(); // try to re-acquire if lost access for some reason
                if (!OK(_device->GetDeviceState(SIZE(dik), &dik)))
                    Zero(dik); // if still failed, then zero
            }
            REPA(Keys) // process only special keys
            {
                C DIK &key = Keys[i];
                Bool on =
                    (dik[key.dik] ||
                     ((key.key == KB_LCTRL)
                          ? _special & 1
                          : GetKeyState(key.key) <
                                0)); // use a combination of both DirectInput and
                                     // WinApi, because DirectInput loses state when
                                     // changing exclusive mode (calling 'Unacquire' and
                                     // 'Acquire'), however we can't use 'GetKeyState'
                                     // for LeftControl (because it can be triggered by
                                     // AltGr and may be disabled by Ctrl+Shift system
                                     // shortcut)
                if (on != FlagOn(_button[key.key], BS_ON)) {
                    if (on)
                        push(key.key, key.scan_code);
                    else
                        release(key.key);
                }
            }
        } else            // if most recent state wasn't checked
            if (_special) // if we're forcing keys, then check if any got released
            {
                if ((_special & 1) && GetKeyState(VK_LCONTROL) >= 0) {
                    release(KB_LCTRL);
                    FlagDisable(_special, 1);
                }
                if ((_special & 2) && GetKeyState(VK_RCONTROL) >= 0) {
                    release(KB_RCTRL);
                    FlagDisable(_special, 2);
                }
                if ((_special & 4) && GetKeyState(VK_LSHIFT) >= 0) {
                    release(KB_LSHIFT);
                    FlagDisable(_special, 4);
                }
                if ((_special & 8) && GetKeyState(VK_RSHIFT) >= 0) {
                    release(KB_RSHIFT);
                    FlagDisable(_special, 8);
                }
                if ((_special & 16) && GetKeyState(VK_LMENU) >= 0) {
                    release(KB_LALT);
                    FlagDisable(_special, 16);
                }
                if ((_special & 32) && GetKeyState(VK_RMENU) >= 0) {
                    release(KB_RALT);
                    FlagDisable(_special, 32);
                }
                if ((_special & 64) && GetKeyState(VK_LWIN) >= 0) {
                    release(KB_LWIN);
                    FlagDisable(_special, 64);
                }
                if ((_special & 128) && GetKeyState(VK_RWIN) >= 0) {
                    release(KB_RWIN);
                    FlagDisable(_special, 128);
                }
            }
    }
#endif
#elif WINDOWS_NEW
    if (App.active()) // need to manually check for certain keys
    {
        // not detected through system events
        Bool print = FlagOn(
            (Int)App.window()->GetKeyState(Windows::System::VirtualKey::Snapshot),
            (Int)Windows::UI::Core::CoreVirtualKeyStates::Down);
        if (print != b(KB_PRINT)) {
            if (print)
                push(KB_PRINT, 0);
            else
                release(KB_PRINT);
        }
    }
#else
    // keyboard state obtained externally in main loop
#endif

#if ANDROID
    if (InputTextIs) {
        Bool reset = false;
        Int enters = 0; // enter workaround
        {
            SyncLocker locker(InputTextLock);
            if (Gui.kb()) {
                Bool remove_enters =
                    Gui.kb()
                        ->isTextLine(); // some soft keyboards (GBoard) may generate
                                        // enter keys as characters, this is a
                                        // workaround for that, it will remove for
                                        // 'TextLine'. For 'TextBox' not needed because
                                        // we allow enters there as characters
                REPA(InputTextData.text) {
                    Char c = InputTextData.text[i];
                    if (!Safe(c) || c == '\n' && remove_enters) {
                        reset = true;
                        if (c == '\n')
                            enters++;                 // increase enter counter
                        InputTextData.text.remove(i); // remove this character
                        if (InputTextData.cur.x > i)
                            InputTextData.cur.x--; // adjust cursor
                        if (InputTextData.cur.y > i)
                            InputTextData.cur.y--; // adjust cursor
                    }
                }
                if (ScreenKeyboard::Set(InputTextData.text) ||
                    !enters) // adjust cursor only if we've changed some text or didn't
                             // process the enter workaround (this is to avoid when
                             // pressing just enter key, changes the cursor position on
                             // GBoard)
                    ScreenKeyboard::Set(InputTextData.cur.y, InputTextData.cur.x);
            }
            _key_buffer_len =
                0; // this is a workaround for a bug in Google/Samsung Keyboard (but
                   // not SwiftKey) when Backspace key is triggered even though it
                   // shouldn't, when tapping Back key on the soft keyboard (when last
                   // character is space, or sometimes when just typed something), in
                   // that case 2 Back's are processed (one from EditText
                   // Java_com_esenthel_Native_text and one AINPUT_EVENT_TYPE_KEY),
                   // this code removes all queued keys to remove KB_BACK
            InputTextIs = false;
        }
        if (reset) // call after we got out of sync lock in case this method would
                   // trigger 'Java_com_esenthel_Native_text' and introduce some
                   // sort of deadlock
        {
            REP(enters)
            queue(KB_ENTER, u'\n');
            resetTextInput();
        }
    }
#endif

    // cursor visibility
    if (_cur >= 0) {
        _cur_hidden = false;
        _curh_t = 0;
    } else if ((_curh_t += Time.ad()) > _curh_tn) {
        if (_curh_t >= 2 * _curh_tn)
            _curh_t = 0;
        else
            _curh_t -= _curh_tn;
        _cur_hidden ^= 1;
    }

    // misc
    setModifiers();
#if ANDROID
    KeySource = KEY_ANY; // re-allow input from all modes
#endif
    nextInQueue(); // process queue before checking for keys below, so they can be
                   // eaten
    if (b(KB_LALT) && !ctrl() && !win()) {
#if DESKTOP
        if (!(App.flag & APP_NO_CLOSE) && bp(KB_F4) && !shift()) {
            App.close();
            eat(KB_F4);
        } // simulate Alt+F4    behavior on (this is also needed on Windows)
#endif
        if ((App.flag & APP_FULL_TOGGLE) && bp(KB_ENTER)) {
            D.toggle(shift());
            eat(KB_ENTER);
        } // process  Alt+Enter to toggle full-screen
    }
}
/******************************************************************************/
// EAT
/******************************************************************************

   do not advance the buffers, so remaining keys will be processed in the next
frame, instead just clear current. This is because the code that checked for
next key press was already executed in this frame, and because in each frame
buffers are automatically advanced, then the key would disappear on next advance
before the codes can check for it disable button flags (such as push), this is
needed so when calling this method, it will prevent other Keyboard Shortcuts
being triggered (those that are detected based on button flags)

/******************************************************************************/
void KeyboardKey::eat() C {
    if (c == Kb.k.c)
        Kb.k.c = '\0';
    if (k == Kb.k.k) {
        FlagDisable(Kb._button[k],
                    BS_NOT_ON); // do this first, while 'k' is still available
                                // (before clearing, in case this is 'Kb.k')
        Kb.k.k = KB_NONE;
    }
}
void KeyboardClass::eat(Char8 c) { eat(Char8To16(c)); }
void KeyboardClass::eat(Char c) {
    if (T.k(c) && c) {
        FlagDisable(_button[k.k],
                    BS_NOT_ON); // do this first, while 'k' is still available
        k.clear();
    }
}
void KeyboardClass::eat(KB_KEY key) {
    if (InRange(key, _button)) {
        FlagDisable(_button[key], BS_NOT_ON); // always disable even if "T.k!=key"
        if (T.k(key) && key)
            k.clear();
    }
}
void KeyboardClass::eatKey() {
    FlagDisable(_button[k.k],
                BS_NOT_ON); // do this first, while 'k' is still available
    k.clear();
}
void KeyboardClass::eat() {
    REPA(_button)
    FlagDisable(_button[i], BS_NOT_ON);
    k.clear();
}
/******************************************************************************/
void KeyboardClass::nextInQueue() {
    if (!_key_buffer_len)
        k.clear();
    else {
        k = _key_buffer[_key_buffer_pos++];
        _key_buffer_len--;
    }
}
KeyboardKey *KeyboardClass::nextKeyPtr() {
    return _key_buffer_len ? &_key_buffer[_key_buffer_pos] : null;
}
void KeyboardClass::nextKey() {
    // eatKey(); instead of calling this, just disable button state, because 'k'
    // will be modified in 'nextInQueue'
    FlagDisable(_button[k.k],
                BS_NOT_ON); // do this first, while 'k' is still available
    nextInQueue();
}
void KeyboardClass::queue(
    C KeyboardKey &key) // !! Warning: this doesn't check for 'key.k' and
                        // 'key.c' being invalid !!
{
    if (_key_buffer_len < 255) {
        _key_buffer[(_key_buffer_pos + _key_buffer_len) & 0xFF] = key;
        _key_buffer_len++;
        _last_key_scan_code = -1;
    }
}
/******************************************************************************/
void KeyboardClass::acquire(Bool on) {
#if WINDOWS_OLD
#if KB_DIRECT_INPUT
    if (_device) {
        if (KEYBOARD_MODE == FOREGROUND) // we need to change acquire only if we're
                                         // operating in Foreground mode
        {
            if (on) {
                _device->Acquire();
                // upon activating the app, we need to check if some keys are pressed,
                // for some reason 'GetDeviceState' will not return it until the next
                // frame
                if (GetKeyState(VK_LCONTROL) < 0 && GetKeyState(VK_RMENU) >= 0) {
                    push(KB_LCTRL, 29);
                    _special |= 1;
                } // we can enable LCTRL only if we know the right alt isn't pressed,
                  // because AltGr (Polish, Norwegian, .. keyboards) generates a false
                  // LCTRL
                if (GetKeyState(VK_RCONTROL) < 0) {
                    push(KB_RCTRL, 29);
                    _special |= 2;
                }
                if (GetKeyState(VK_LSHIFT) < 0) {
                    push(KB_LSHIFT, 42);
                    _special |= 4;
                }
                if (GetKeyState(VK_RSHIFT) < 0) {
                    push(KB_RSHIFT, 54);
                    _special |= 8;
                }
                if (GetKeyState(VK_LMENU) < 0) {
                    push(KB_LALT, 56);
                    _special |= 16;
                }
                if (GetKeyState(VK_RMENU) < 0) {
                    push(KB_RALT, 56);
                    _special |= 32;
                }
                if (GetKeyState(VK_LWIN) < 0) {
                    push(KB_LWIN, 91);
                    _special |= 64;
                }
                if (GetKeyState(VK_RWIN) < 0) {
                    push(KB_RWIN, 92);
                    _special |= 128;
                }
            } else
                _device->Unacquire();
        } else {
            // ignore recorded background input (this will remove keys from the DI
            // buffer)
            DIDEVICEOBJECTDATA didod[BUF_KEYS];
            DWORD elms = BUF_KEYS, ret = _device->GetDeviceData(
                                       SIZE(DIDEVICEOBJECTDATA), didod, &elms, 0);
        }
    }
#elif KB_RAW_INPUT
    if (KEYBOARD_MODE == FOREGROUND &&
        on) { // upon activating the app, we need to check if some keys are
              // pressed, for some reason they will not be reported until the next
              // frame
        if (GetKeyState(VK_LCONTROL) < 0 && GetKeyState(VK_RMENU) >= 0)
            push(KB_LCTRL, 29); // we can enable LCTRL only if we know the right alt
                                // isn't pressed, because AltGr (Polish, Norwegian, ..
                                // keyboards) generates a false LCTRL
        if (GetKeyState(VK_RCONTROL) < 0)
            push(KB_RCTRL, 29);
        if (GetKeyState(VK_LSHIFT) < 0)
            push(KB_LSHIFT, 42);
        if (GetKeyState(VK_RSHIFT) < 0)
            push(KB_RSHIFT, 54);
        if (GetKeyState(VK_LMENU) < 0)
            push(KB_LALT, 56);
        if (GetKeyState(VK_RMENU) < 0)
            push(KB_RALT, 56);
        if (GetKeyState(VK_LWIN) < 0)
            push(KB_LWIN, 91);
        if (GetKeyState(VK_RWIN) < 0)
            push(KB_RWIN, 92);
    }
#endif
#endif
#if WINDOWS || LINUX
    if (!on)
        REP(256)
    release(KB_KEY(
        i)); // need to manually release because Windows 'WM_*KEYUP', 'KeyUp'
             // and Linux 'KeyRelease' aren't processed when app lost focus
#endif
}
void KeyboardClass::exclusive(Bool on) {
#if WINDOWS_OLD
    if (_exclusive != on) {
        _exclusive = on; // set this first because it affects 'KEYBOARD_MODE'
#if KB_RAW_INPUT
        if (App.window()) {
            RAWINPUTDEVICE rid[1];

            rid[0].usUsagePage = 0x01;
            rid[0].usUsage = 0x06; // keyboard
            rid[0].dwFlags = ((KEYBOARD_MODE == BACKGROUND) ? RIDEV_INPUTSINK : 0) |
                             (_exclusive ? RIDEV_NOHOTKEYS : 0);
            rid[0].hwndTarget = App.window();

            RegisterRawInputDevices(rid, Elms(rid), SIZE(RAWINPUTDEVICE));
        }
#elif KB_DIRECT_INPUT
        if (_device) {
            _device->Unacquire(); // this also resets the 'GetDeviceState' of any
                                  // currently pressed keys, they need to be pushed
                                  // again to activate their state
            _device->SetCooperativeLevel(
                App.window(), (_exclusive ? DISCL_NOWINKEY : 0) | DISCL_NONEXCLUSIVE |
                                  ((KEYBOARD_MODE == FOREGROUND) ? DISCL_FOREGROUND
                                                                 : DISCL_BACKGROUND));
            if (KEYBOARD_MODE == BACKGROUND || App.active())
                _device->Acquire(); // in background mode we always want the keyboard to
                                    // be acquired, in foreground only if it's active
            // because calling 'Unacquire' resets the state, we need to remember if
            // some keys are pressed, other keys don't have to be remembered because
            // they are processed using WM_*KEY*
            _special = 1 * Kb.b(KB_LCTRL) | 2 * Kb.b(KB_RCTRL) | 4 * Kb.b(KB_LSHIFT) |
                       8 * Kb.b(KB_RSHIFT) | 16 * Kb.b(KB_LALT) | 32 * Kb.b(KB_RALT) |
                       64 * Kb.b(KB_LWIN) | 128 * Kb.b(KB_RWIN);
        }
#endif
    }
#endif
}
/******************************************************************************/
KB_KEY KeyboardClass::qwerty(KB_KEY qwerty) C {
    ASSERT(1 << (8 * SIZE(qwerty)) == ELMS(_qwerty));
    return _qwerty[qwerty];
}

Bool KeyboardClass::rect(Rect &rect) C {
    if (_visible) {
        rect = D.pixelToScreen(_recti);
        return true;
    }
    return false;
}
void KeyboardClass::screenChanged() {
    if (screen_changed)
        screen_changed();
    Gui.updateTextMenu();
}
void KeyboardClass::set(C RectI &rect) {
    if (_recti != rect) {
        _recti = rect;
        screenChanged();
    }
}
void ScreenKeyboard::set() {
    text = null;
    start = end = 0;
    max_length = -1;
    multi_line = pass = number = email = url = false;

    if (Gui.kb())
        switch (Gui.kb()->type()) {
        case GO_TEXTBOX: {
            C TextBox &tb = Gui.kb()->asTextBox();
            text = &tb();
            end = tb.cursor();
            start = ((tb._edit.sel < 0) ? tb.cursor() : tb._edit.sel);
            multi_line = true;
            max_length = tb.maxLength();
        } break;

        case GO_TEXTLINE: {
            C TextLine &tl = Gui.kb()->asTextLine();
            text = &tl();
            pass = tl.password();
            number = tl.number();
            email = tl.email();
            url = tl.url();
            end = tl.cursor();
            start = ((tl._edit.sel < 0) ? tl.cursor() : tl._edit.sel);
            max_length = tl.maxLength();
        } break;
        }
}
Bool ScreenKeyboard::Set(CChar *text) {
    if (Gui.kb())
        switch (Gui.kb()->type()) {
        case GO_TEXTBOX: {
            TextBox &tb = Gui.kb()->asTextBox();
            return tb.setChanged(text);
        } break;

        case GO_TEXTLINE: {
            TextLine &tl = Gui.kb()->asTextLine();
            return tl.setChanged(text);
        } break;
        }
    return false;
}
Bool ScreenKeyboard::Set(Int cur, Int sel) {
    if (Gui.kb())
        switch (Gui.kb()->type()) {
        case GO_TEXTBOX: {
            TextBox &tb = Gui.kb()->asTextBox();
            tb._edit.sel = ((sel == cur) ? -1 : sel);
            return tb.cursorChanged(cur);
        } break;

        case GO_TEXTLINE: {
            TextLine &tl = Gui.kb()->asTextLine();
            tl._edit.sel = ((sel == cur) ? -1 : sel);
            return tl.cursorChanged(cur);
        } break;
        }
    return false;
}
#if ANDROID
static jint SKMode(C ScreenKeyboard &sk) {
    return (sk.pass) | (sk.number << 1) | (sk.email << 2) | (sk.url << 3);
}
static Bool KBWantVisible;
#endif
void KeyboardClass::setVisible() {
    Bool visible = (Gui.kb() && Gui.kb()->isTextEdit());
#if WINDOWS_OLD
    imm(visible); // here ignore 'hardware'
#endif
#if !SWITCH                 // on Switch always show, because hardware keyboard is limited to \
            // simple US-QWERTY without support of other languages
    visible &= !hardware(); // show only if hardware unavailable
#endif

#if WINDOWS_NEW || ANDROID || IOS || SWITCH
    ScreenKeyboard sk;
    if (visible)
        sk.set();
#if WINDOWS_NEW
    if (auto input_pane =
            Windows::UI::ViewManagement::InputPane::GetForCurrentView()) {
        if (visible) {
            if (TextEditContext) {
                TextEditContext->InputScope =
                    (sk.url ? Windows::UI::Text::Core::CoreTextInputScope::Url
                     : sk.email
                         ? Windows::UI::Text::Core::CoreTextInputScope::EmailAddress
                     : sk.number
                         ? (sk.pass
                                ? Windows::UI::Text::Core::CoreTextInputScope::
                                      PasswordNumeric
                                : Windows::UI::Text::Core::CoreTextInputScope::Number)
                         : (sk.pass
                                ? Windows::UI::Text::Core::CoreTextInputScope::Password
                                : Windows::UI::Text::Core::CoreTextInputScope::Text));
                TextEditContext->NotifyFocusEnter();
            }
            input_pane->TryShow();
        } else {
            if (TextEditContext)
                TextEditContext->NotifyFocusLeave();
            input_pane->TryHide();
        }
    }
#elif ANDROID
    if (Jni && ActivityClass && Activity) {
        if (visible) {
            KBWantVisible = true;
            if (JMethodID editText =
                    Jni.func(ActivityClass, "editText", "(Ljava/lang/String;III)V"))
                if (JString t = JString(Jni, sk.text ? *sk.text : S))
                    Jni->CallVoidMethod(Activity, editText, t(), jint(sk.start),
                                        jint(sk.end), SKMode(sk));
        } else if (KBWantVisible) {
            KBWantVisible = false;
            if (JMethodID editTextHide =
                    Jni.func(ActivityClass, "editTextHide", "()V"))
                Jni->CallVoidMethod(Activity, editTextHide);
        }
    }
#elif IOS
    if (EAGLView *view = GetUIView()) {
        if (visible) {
            // view.keyboardType=(sk.url ? UIKeyboardTypeURL : sk.email ?
            // UIKeyboardTypeEmailAddress : sk.number ? UIKeyboardTypeNumberPad :
            // UIKeyboardTypeDefault);
        }
        [view keyboardVisible:visible];
    }
#elif SWITCH
    NS::KeyboardVisible(visible ? &sk : null);
#endif
#endif
}
void KeyboardClass::resetTextInput() {
#if WINDOWS_NEW || ANDROID || SWITCH
#if ANDROID
    if (KBWantVisible) // on Android check 'KBWantVisible' because '_visible'
                       // might be changed at a later stage due to JAVA threads,
                       // so immiediately after requesting screen keyboard,
                       // '_visible' might still be false, but we already want to
                       // change it
#else
    if (_visible)
#endif
    {
        ScreenKeyboard sk;
        sk.set();
#if WINDOWS_NEW
        if (TextEditContext) {
            TextEditContext->InputScope =
                (sk.email ? Windows::UI::Text::Core::CoreTextInputScope::EmailAddress
                 : sk.number
                     ? (sk.pass ? Windows::UI::Text::Core::CoreTextInputScope::
                                      PasswordNumeric
                                : Windows::UI::Text::Core::CoreTextInputScope::Number)
                     : (sk.pass
                            ? Windows::UI::Text::Core::CoreTextInputScope::Password
                            : Windows::UI::Text::Core::CoreTextInputScope::Text));
            Windows::UI::Text::Core::CoreTextRange modify_range, selection;
            modify_range.StartCaretPosition = 0;
            modify_range.EndCaretPosition = INT_MAX;
            MinMax(sk.start, sk.end, selection.StartCaretPosition,
                   selection.EndCaretPosition);
            TextEditContext->NotifyTextChanged(
                modify_range, sk.text ? sk.text->length() : 0, selection);
        }
#elif ANDROID
        if (Jni && ActivityClass && Activity)
            if (JMethodID editTextSet = Jni.func(ActivityClass, "editTextSet",
                                                 "(Ljava/lang/String;III)V"))
                if (JString t = JString(Jni, sk.text ? *sk.text : S))
                    Jni->CallVoidMethod(Activity, editTextSet, t(), jint(sk.start),
                                        jint(sk.end), SKMode(sk));
#elif SWITCH
        NS::KeyboardSet(sk);
#endif
    }
#endif
}
/******************************************************************************/
// KEYBOARD SHORTCUT
/******************************************************************************/
#if APPLE
#define KBSC_CTRL_EX (KBSC_CTRL | KBSC_WIN_CTRL)
#define KBSC_WIN_EX (KBSC_WIN | KBSC_CTRL_CMD)
#else
#define KBSC_CTRL_EX (KBSC_CTRL | KBSC_CTRL_CMD)
#define KBSC_WIN_EX (KBSC_WIN | KBSC_WIN_CTRL)
#endif

inline Bool KbSc::testFlag() C {
    return FlagOn(flag, KBSC_CTRL_EX) == Kb.k.ctrl() &&
           FlagOn(flag, KBSC_SHIFT) == Kb.k.shift() &&
           FlagOn(flag, KBSC_ALT) == Kb.k.alt() &&
           FlagOn(flag, KBSC_WIN_EX) == Kb.k.win() &&
           (flag & KBSC_REPEAT || Kb.k.first());
}
inline Bool KbSc::testFlagChar() C {
    return FlagOn(flag, KBSC_CTRL_EX) == Kb.k.ctrl()
           //&& FlagOn(flag, KBSC_SHIFT  )==Kb.k.shift() shift is not checked,
           // because for KBSC_CHAR we just specify the character being lower/upper
           // case, and KBSC_SHIFT would mess this up
           && FlagOn(flag, KBSC_ALT) ==
                  Kb.k.lalt() // only left Alt is checked, because right Alt may
                              // trigger accented characters
           && FlagOn(flag, KBSC_WIN_EX) == Kb.k.win();
}

Bool KbSc::pd() C {
    switch (mode) {
    case KBSC_CHAR:
        return Kb.k.c == index && testFlagChar();
    case KBSC_KEY:
        return Kb.k.k == index &&
               testFlag(); // must check either 'key' or 'bp', but not both, because
                           // 'key' is set to the first key queued in the buffer, if
                           // there are multiple keys then by default they will be
                           // processed one per frame, so if two keys were pressed,
                           // 2nd will be processed on the next frame, and in that
                           // case 'bp' for it is now false. We could check all keys
                           // in the buffer, however it's better to process keyboard
                           // shortcuts in order (for example if there's keyboard
                           // shortcuts: "Save" "Close" and user presses both very
                           // quickly in the same frame, we need to make sure that
                           // they are processed in correct order). 'key' was chosen
                           // instead of 'bp' to allow processing shortcuts in order
    }
    return false;
}

#if APPLE
#define WIN_TEXT "Cmd+" // Apple keyboards have "Cmd" key instead of "Win" key
#else
#define WIN_TEXT "Win+"
#endif
Str KbSc::asText() C {
    Str s;
    switch (mode) {
    case KBSC_CHAR: {
        Char c = index;
        if (flag & KBSC_CTRL_EX)
            s += "Ctrl+";
        if (flag & KBSC_WIN_EX)
            s += WIN_TEXT;
        if (CharFlagFast(c) & CHARF_UP)
            s += "Shift+";
        if (flag & KBSC_ALT)
            s += "Alt+";
        if (CChar8 *n = CharName(c))
            s += n;
        else
            s += CaseUp(c);
    } break;
    case KBSC_KEY: {
        if (flag & KBSC_CTRL_EX)
            s += "Ctrl+";
        if (flag & KBSC_WIN_EX)
            s += WIN_TEXT;
        if (flag & KBSC_SHIFT)
            s += "Shift+";
        if (flag & KBSC_ALT)
            s += "Alt+";
        s += Kb.keySymbol(KB_KEY(index));
    } break;
    }
    return s;
}
void KbSc::eat() C {
    switch (mode) {
    case KBSC_CHAR:
        Kb.eat(Char(index));
        break;
    case KBSC_KEY:
        Kb.eat(KB_KEY(index));
        break;
    }
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
#if ANDROID
extern "C" {

JNIEXPORT void JNICALL Java_com_esenthel_Native_text(JNIEnv *env, jclass clazz,
                                                     jstring text, jint start,
                                                     jint end) {
    JNI jni(env);
    SyncLocker locker(InputTextLock);
    InputTextData.text = jni(text);
    InputTextData.cur.set(start, end);
    InputTextIs = true;
}
JNIEXPORT void JNICALL Java_com_esenthel_Native_key(JNIEnv *env, jclass clazz,
                                                    jint chr, jint key_code) {
    switch (KeySource) {
    case KEY_ANY:
        KeySource = KEY_JAVA; // !! no break on purpose !!
    case KEY_JAVA:
        Kb.queue(Char(chr), key_code);
        break;
    }
}
}
#endif
/******************************************************************************/
