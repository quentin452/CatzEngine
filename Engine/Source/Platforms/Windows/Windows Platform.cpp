/******************************************************************************/
#include "stdafx.h"
#include <Windows.h>
#include <cstdlib>
#include <iostream>
#include <shellapi.h>
#include <stdexcept>
#include <tlhelp32.h>
#include <windows.h>

/******************************************************************************/
namespace EE {
/******************************************************************************/
// Avoid making 70 mb of crash dump "every time" you close the app (avoid https://github.com/quentin452/CatzEngine/issues/24)
LONG DisableMemoryCrashDump() {
    HKEY hKey;
    LONG result;
    DWORD dwDisposition;
    DWORD CustomDumpFlags = 0x41000200;
    DWORD DumpCount = 0;
    DWORD DumpType = 0;

    result = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\Windows Error Reporting\\LocalDumps\\Titan Editor.exe",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey,
        &dwDisposition);

    if (result == ERROR_SUCCESS) {
        result = RegSetValueEx(hKey, L"CustomDumpFlags", 0, REG_DWORD, (BYTE *)&CustomDumpFlags, sizeof(DWORD));
        result = RegSetValueEx(hKey, L"DumpCount", 0, REG_DWORD, (BYTE *)&DumpCount, sizeof(DWORD));
        result = RegSetValueEx(hKey, L"DumpType", 0, REG_DWORD, (BYTE *)&DumpType, sizeof(DWORD));
        RegCloseKey(hKey);
    }

    return result;
}

/*void terminatePreviousInstances() {
    DWORD currentProcessId = GetCurrentProcessId();
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapShot, &pe32)) {
        do {
            wchar_t szExeFileW[MAX_PATH];
            MultiByteToWideChar(CP_ACP, 0, pe32.szExeFile, -1, szExeFileW, MAX_PATH);
            if (_wcsicmp(szExeFileW, L"Titan Editor.exe") == 0 &&
                pe32.th32ProcessID != currentProcessId) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                if (hProcess != NULL) {
                    TerminateProcess(hProcess, 1);
                    CloseHandle(hProcess);
                }
            }
        } while (Process32Next(hSnapShot, &pe32));
    }
    CloseHandle(hSnapShot);
}*/

bool elevatePrivileges(const char *exePath) {
    // Check if the exePath ends with "Titan Editor.exe"
    std::string fileName = exePath;
    if (fileName.size() >= 15 && fileName.substr(fileName.size() - 15) == "Titan Editor.exe") {
        bool isElevated = false;
        HANDLE hToken = nullptr;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            TOKEN_ELEVATION Elevation;
            DWORD dwSize;
            if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &dwSize)) {
                isElevated = Elevation.TokenIsElevated;
            }
        }
        if (!isElevated) {
            SHELLEXECUTEINFOA shExInfo = {0};
            shExInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
            shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
            shExInfo.lpVerb = "runas";
            shExInfo.lpFile = exePath;
            shExInfo.lpParameters = "";
            shExInfo.nShow = SW_SHOW;
            if (ShellExecuteExA(&shExInfo)) {
                WaitForSingleObject(shExInfo.hProcess, INFINITE);
                CloseHandle(shExInfo.hProcess);
                return true;
            }
        }
        return isElevated;
    } else {
        // If the filename does not match, assume already elevated
        return true;
    }
}
void InitThreadedLoggerForCPP() {
#ifdef _WIN32
    LoggerGlobals::UsernameDirectory = std::getenv("USERNAME");
#else
    LoggerGlobals::UsernameDirectory = std::getenv("USER");
#endif

    // this is the folder that contain your src files like main.cpp
    LoggerGlobals::SrcProjectDirectory = "CatzEngine";
    // Create Log File and folder
    LoggerGlobals::LogFolderPath = "C:\\Users\\" +
                                   LoggerGlobals::UsernameDirectory +
                                   "\\.CatzEngine\\logging\\";
    LoggerGlobals::LogFilePath =
        "C:\\Users\\" + LoggerGlobals::UsernameDirectory +
        "\\.CatzEngine\\logging\\CatzEngine.log";
    LoggerGlobals::LogFolderBackupPath =
        "C:\\Users\\" + LoggerGlobals::UsernameDirectory +
        "\\.CatzEngine\\logging\\LogBackup";
    LoggerGlobals::LogFileBackupPath =
        "C:\\Users\\" + LoggerGlobals::UsernameDirectory +
        "\\.CatzEngine\\logging\\LogBackup\\CatzEngine-";

    GlobalsLoggerInstance::LoggerInstance.StartLoggerThread(
        LoggerGlobals::LogFolderPath, LoggerGlobals::LogFilePath,
        LoggerGlobals::LogFolderBackupPath, LoggerGlobals::LogFileBackupPath);
}
#if WINDOWS_NEW
using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Devices;
using namespace Windows::Devices::Input;
using namespace Windows::Devices::Sensors;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Gaming::Input;
using namespace Windows::Graphics::Display;
using namespace Windows::System;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::ViewManagement;

// must hold global refs, otherwise events will not be called
static Sensors::Accelerometer ^ accelerometer;
static Sensors::Gyrometer ^ gyrometer;
static Sensors::Magnetometer ^ magnetometer;

static EventRegistrationToken MagnetometerToken, MouseToken;
static TypedEventHandler<Sensors::Magnetometer ^, MagnetometerReadingChangedEventArgs ^> ^ MagnetometerHandler;

static Windows::Devices::Enumeration::DeviceWatcher ^ DeviceWatcher;

#if JP_GAMEPAD_INPUT
static Int FindJoypadI(Windows::Gaming::Input::Gamepad ^ gamepad) {
    REPA(Joypads)
    if (Joypads[i]._gamepad == gamepad)
        return i;
    return -1;
}
static Int FindJoypadI(Windows::Gaming::Input::RawGameController ^ raw_game_controller) {
    REPA(Joypads)
    if (Joypads[i]._raw_game_controller == raw_game_controller)
        return i;
    return -1;
}
static Int FindJoypadI(Windows::Gaming::Input::IGameController ^ controller) {
    REPA(Joypads) {
        Joypad &jp = Joypads[i];
        if (jp._gamepad == controller || jp._raw_game_controller == controller)
            return i;
    }
    return -1;
}

struct GamepadChange {
    Bool added;
    Gamepad ^ gamepad;
    RawGameController ^ raw_game_controller;

    void process();
};
static MemcThreadSafe<GamepadChange> GamepadChanges;

static void GamepadChanged() {
    MemcThreadSafeLock lock(GamepadChanges);
    FREPA(GamepadChanges)
    GamepadChanges.lockedElm(i).process();
    GamepadChanges.lockedClear(); // process in order
}
static void GamepadChanged(Bool added, Gamepad ^ gamepad, RawGameController ^ raw_game_controller) {
    {
        MemcThreadSafeLock lock(GamepadChanges);
        GamepadChange &gpc = GamepadChanges.lockedNew();
        gpc.added = added;
        gpc.gamepad = gamepad;
        gpc.raw_game_controller = raw_game_controller;
    }
    App.addFuncCall(GamepadChanged);
}
#endif
/******************************************************************************/
static void Kb_push(KB_KEY key, Int scan_code) {
    Kb.push(key, scan_code);
    // !! queue characters after push !!
    if (Kb.anyCtrl() && !Kb.anyAlt()) // if Control with no Alt is on, then 'OnCharacterReceived' will not be called, so we must add this char here
    {
        if (key >= 'A' && key <= 'Z')
            Kb.queue(Char(key + (Kb.anyShift() ? 0 : 'a' - 'A')), scan_code);
        else if (key >= '0' && key <= '9')
            Kb.queue(Char(key), scan_code);
    }
}
struct AppEvent {
    enum TYPE : Byte {
        KEY_DOWN,
        CHAR,
        KEY_UP,
    };

    TYPE type;
    union {
        Char c;
        KB_KEY key;
        Byte scan_code;
    };

    void keyDown(KB_KEY key, Byte scan_code) {
        type = KEY_DOWN;
        T.key = key;
        T.scan_code = scan_code;
    }
    void chr(Char c, Byte scan_code) {
        type = CHAR;
        T.c = c;
        T.scan_code = scan_code;
    }
    void keyUp(KB_KEY key) {
        type = KEY_UP;
        T.key = key;
    }

    void execute() {
        switch (type) {
        case KEY_DOWN:
            Kb_push(key, scan_code);
            break;
        case CHAR:
            Kb.queue(c, scan_code);
            break;
        case KEY_UP:
            Kb.release(key);
            break;
        }
    }

    AppEvent() {} // needed because of union
};
static Memc<AppEvent> Events;
void Application::ExecuteRecordedEvents() {
    FREPAO(Events).execute(); // run in order
    Events.clear();
}
/******************************************************************************/
static Bool MouseButton[5]; // keep own mouse button state, because 'Ms.b' may not be most recent, because it can have a queue
static void UpdateMouseButton(Windows::UI::Input::PointerPointProperties ^ properties) {
    Bool state[] =
        {
            properties->IsLeftButtonPressed,
            properties->IsRightButtonPressed,
            properties->IsMiddleButtonPressed,
            properties->IsXButton1Pressed,
            properties->IsXButton2Pressed,
        };
    ASSERT(Elms(state) == Elms(MouseButton));
    FREPA(MouseButton)
    if (MouseButton[i] != state[i]) {
        if (MouseButton[i] ^= 1)
            Ms.push(i);
        else
            Ms.release(i);
    }
}
/******************************************************************************/
Bool Application::Fullscreen() { return Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->IsFullScreenMode; }

void SetMagnetometerRefresh(Flt interval) {
    if (magnetometer) {
        magnetometer->ReadingChanged -= MagnetometerToken;
        if (interval >= 0) // enable
        {
            magnetometer->ReportInterval = Max(RoundU(interval * 1000), accelerometer->MinimumReportInterval);
            MagnetometerToken = (magnetometer->ReadingChanged += MagnetometerHandler);
        } else {
            magnetometer->ReportInterval = 0;
        }
    }
}
static void DeviceChanged() { InputDevices.checkMouseKeyboard(); }

#define KEY_EVENTS 1 // 1=better (can catch key events that occur when ALT is pressed) 0=(can't catch ALT+keys)

ref struct FrameworkView sealed : IFrameworkView {
    // IFrameworkView methods
    virtual void Initialize(CoreApplicationView ^ applicationView) {
        applicationView->Activated += ref new TypedEventHandler<CoreApplicationView ^, IActivatedEventArgs ^>(this, &FrameworkView::OnActivated);
        CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs ^>(this, &FrameworkView::OnSuspending);
        CoreApplication::Resuming += ref new EventHandler<Object ^>(this, &FrameworkView::OnResuming);

        Windows::System::MemoryManager::AppMemoryUsageIncreased += ref new EventHandler<Object ^>(this, &FrameworkView::OnAppMemoryUsageIncreased);
    }
    virtual void SetWindow(CoreWindow ^ window) // called before 'Load'
    {
        App.window().set(window);

        DeviceWatcher = Windows::Devices::Enumeration::DeviceInformation::CreateWatcher();
        DeviceWatcher->Added += ref new TypedEventHandler<Windows::Devices::Enumeration::DeviceWatcher ^, Windows::Devices::Enumeration::DeviceInformation ^>(this, &FrameworkView::OnDeviceAdded);
        DeviceWatcher->Removed += ref new TypedEventHandler<Windows::Devices::Enumeration::DeviceWatcher ^, Windows::Devices::Enumeration::DeviceInformationUpdate ^>(this, &FrameworkView::OnDeviceUpdate);
        DeviceWatcher->Updated += ref new TypedEventHandler<Windows::Devices::Enumeration::DeviceWatcher ^, Windows::Devices::Enumeration::DeviceInformationUpdate ^>(this, &FrameworkView::OnDeviceUpdate);
        DeviceWatcher->Start();

        DisplayInformation ^ display_info = DisplayInformation::GetForCurrentView();
        ScreenScale = display_info->RawPixelsPerViewPixel;

        window->SizeChanged += ref new TypedEventHandler<CoreWindow ^, WindowSizeChangedEventArgs ^>(this, &FrameworkView::OnWindowSizeChanged);
        window->ResizeCompleted += ref new TypedEventHandler<CoreWindow ^, Object ^>(this, &FrameworkView::OnResizeCompleted);
        window->VisibilityChanged += ref new TypedEventHandler<CoreWindow ^, VisibilityChangedEventArgs ^>(this, &FrameworkView::OnVisibilityChanged);
        window->Closed += ref new TypedEventHandler<CoreWindow ^, CoreWindowEventArgs ^>(this, &FrameworkView::OnWindowClosed);
        window->Activated += ref new TypedEventHandler<CoreWindow ^, WindowActivatedEventArgs ^>(this, &FrameworkView::OnWindowActivated);

        display_info->DpiChanged += ref new TypedEventHandler<DisplayInformation ^, Object ^>(this, &FrameworkView::OnDpiChanged);
        display_info->OrientationChanged += ref new TypedEventHandler<DisplayInformation ^, Object ^>(this, &FrameworkView::OnOrientationChanged);
        display_info->ColorProfileChanged += ref new TypedEventHandler<DisplayInformation ^, Object ^>(this, &FrameworkView::OnColorProfileChanged);
        display_info->AdvancedColorInfoChanged += ref new TypedEventHandler<DisplayInformation ^, Object ^>(this, &FrameworkView::OnAdvancedColorInfoChanged);
        DisplayInformation::DisplayContentsInvalidated += ref new TypedEventHandler<DisplayInformation ^, Object ^>(this, &FrameworkView::OnDisplayContentsInvalidated);

        MouseToken = (MouseDevice::GetForCurrentView()->MouseMoved += ref new TypedEventHandler<MouseDevice ^, MouseEventArgs ^>(this, &FrameworkView::OnMouseMoved));

        window->PointerMoved += ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &FrameworkView::OnPointerMoved);
        window->PointerEntered += ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &FrameworkView::OnPointerEntered);
        window->PointerExited += ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &FrameworkView::OnPointerExited);
        window->PointerCaptureLost += ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &FrameworkView::OnPointerExited);
        window->PointerPressed += ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &FrameworkView::OnPointerPressed);
        window->PointerReleased += ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &FrameworkView::OnPointerReleased);
        window->PointerWheelChanged += ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &FrameworkView::OnPointerWheelChanged);

#if KEY_EVENTS
        window->Dispatcher->AcceleratorKeyActivated += ref new TypedEventHandler<CoreDispatcher ^, AcceleratorKeyEventArgs ^>(this, &FrameworkView::OnAcceleratorKeyActivated);
#else
        window->KeyDown += ref new TypedEventHandler<CoreWindow ^, KeyEventArgs ^>(this, &FrameworkView::OnKeyDown);
        window->KeyUp += ref new TypedEventHandler<CoreWindow ^, KeyEventArgs ^>(this, &FrameworkView::OnKeyUp);
        window->CharacterReceived += ref new TypedEventHandler<CoreWindow ^, CharacterReceivedEventArgs ^>(this, &FrameworkView::OnCharacterReceived);
#endif

        Gamepad::GamepadAdded += ref new EventHandler<Gamepad ^>(this, &FrameworkView::OnGamepadAdded);
        Gamepad::GamepadRemoved += ref new EventHandler<Gamepad ^>(this, &FrameworkView::OnGamepadRemoved);

        RawGameController::RawGameControllerAdded += ref new EventHandler<RawGameController ^>(this, &FrameworkView::OnRawGameControllerAdded);
        RawGameController::RawGameControllerRemoved += ref new EventHandler<RawGameController ^>(this, &FrameworkView::OnRawGameControllerRemoved);

        if (accelerometer = Accelerometer::GetDefault()) {
            accelerometer->ReportInterval = Max(16, accelerometer->MinimumReportInterval);
            accelerometer->ReadingChanged += ref new TypedEventHandler<Sensors::Accelerometer ^, AccelerometerReadingChangedEventArgs ^>(this, &FrameworkView::OnAccelerometerChanged);
        }
        if (gyrometer = Gyrometer::GetDefault()) {
            gyrometer->ReportInterval = Max(16, gyrometer->MinimumReportInterval);
            gyrometer->ReadingChanged += ref new TypedEventHandler<Sensors::Gyrometer ^, GyrometerReadingChangedEventArgs ^>(this, &FrameworkView::OnGyrometerChanged);
        }
        if (magnetometer = Magnetometer::GetDefault()) {
            MagnetometerHandler = ref new TypedEventHandler<Sensors::Magnetometer ^, MagnetometerReadingChangedEventArgs ^>(this, &FrameworkView::OnMagnetometerChanged);
        }
        // SystemNavigationManager::GetForCurrentView()->AppViewBackButtonVisibility = Windows::UI::Core::AppViewBackButtonVisibility::Visible; // display back button on app title bar
        SystemNavigationManager::GetForCurrentView()->BackRequested += ref new EventHandler<BackRequestedEventArgs ^>(this, &FrameworkView::OnBackRequested);

        ApplicationView::GetForCurrentView()->SetPreferredMinSize(Size(Max(1, PixelsToDips(1)), Max(1, PixelsToDips(1)))); // using <1 means to use system default min size, so use Max 1 to always set a custom size

        if (auto manager = Windows::UI::Text::Core::CoreTextServicesManager::GetForCurrentView()) {
            TextEditContext = manager->CreateEditContext();
            TextEditContext->InputPaneDisplayPolicy = Windows::UI::Text::Core::CoreTextInputPaneDisplayPolicy::Manual;
            TextEditContext->InputScope = Windows::UI::Text::Core::CoreTextInputScope::Text;
            TextEditContext->FocusRemoved += ref new TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext ^, Platform::Object ^>(this, &FrameworkView::OnInputFocusRemoved);
            TextEditContext->TextRequested += ref new TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext ^, Windows::UI::Text::Core::CoreTextTextRequestedEventArgs ^>(this, &FrameworkView::OnInputTextRequested);
            TextEditContext->SelectionRequested += ref new TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext ^, Windows::UI::Text::Core::CoreTextSelectionRequestedEventArgs ^>(this, &FrameworkView::OnInputSelectionRequested);
            TextEditContext->TextUpdating += ref new TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext ^, Windows::UI::Text::Core::CoreTextTextUpdatingEventArgs ^>(this, &FrameworkView::OnInputTextUpdating);
            TextEditContext->SelectionUpdating += ref new TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext ^, Windows::UI::Text::Core::CoreTextSelectionUpdatingEventArgs ^>(this, &FrameworkView::OnInputSelectionUpdating);
            // TextEditContext->LayoutRequested    += ref new TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext^, Windows::UI::Text::Core::CoreTextLayoutRequestedEventArgs   ^>(this, &FrameworkView::OnInputLayoutRequested);
        }
        if (auto input_pane = InputPane::GetForCurrentView()) {
            input_pane->Showing += ref new TypedEventHandler<InputPane ^, InputPaneVisibilityEventArgs ^>(this, &FrameworkView::OnInputPaneShowing);
            input_pane->Hiding += ref new TypedEventHandler<InputPane ^, InputPaneVisibilityEventArgs ^>(this, &FrameworkView::OnInputPaneHiding);
        }

        setOrientation(display_info->CurrentOrientation, display_info->NativeOrientation); // !! call this before 'setMode' !!
        setMode();
    }
    virtual void Load(Platform::String ^ entryPoint) // called after 'SetWindow'
    {
    }
    virtual void Run() // changing full screen mode 'TryEnterFullScreenMode' will work only here
    {
        if (!App._closed) // only if app didn't call 'Exit'
        {
            if (!App.create())
                App._close = true; // put app create here because only at this stage 'RequestDisplayMode' will work
            App.loop();
        }
        MouseDevice::GetForCurrentView()->MouseMoved -= MouseToken; // !! without this, app will crash on Windows Phone when closing the app !!
    }
    virtual void Uninitialize() // this is called only when app requested to finish (and NOT when user clicked the close box)
    {
        if (App._closed)
            return; // do nothing if app called 'Exit'
        App.del();
    }

    // custom callbacks
    void OnActivated(CoreApplicationView ^ applicationView, IActivatedEventArgs ^ args) {
        CoreWindow::GetForCurrentThread()->Activate();
    }
    void OnSuspending(Object ^ sender, SuspendingEventArgs ^ args) {
        if (App._closed)
            return; // do nothing if app called 'Exit'
        SuspendingDeferral ^ deferral = args->SuspendingOperation->GetDeferral();
        create_task([this, deferral]() {
            if (!App._closed) // only if app didn't call 'Exit', check this again in case 'Exit' was called later
            {
                if (App.save_state)
                    App.save_state(); // call this first as it's most important
                                      // App.setActive(false); just use 'OnWindowActivated'
                PauseSound();
                App.lowMemory(); // reduce memory usage (this is optional)
            }

            if (D3D) {
                IDXGIDevice3 *device;
                if (OK(D3D->QueryInterface(__uuidof(IDXGIDevice3), (Ptr *)&device))) {
                    device->Trim();
                    device->Release();
                }
            }

            deferral->Complete();
        });
    }
    void OnResuming(Object ^ sender, Object ^ args) {
        if (App._closed)
            return; // do nothing if app called 'Exit'
        ResumeSound();
        // App.setActive(true); just use 'OnWindowActivated'
    }
    void OnAppMemoryUsageIncreased(Object ^ sender, Object ^ args) {
        if (MemoryManager::AppMemoryUsageLevel == AppMemoryUsageLevel::OverLimit)
            App.lowMemory();
    }
    void OnWindowSizeChanged(CoreWindow ^ sender, WindowSizeChangedEventArgs ^ args) {
        setMode();
    }
    void OnResizeCompleted(CoreWindow ^ sender, Object ^ args) {
        setMode();
    }
    void OnVisibilityChanged(CoreWindow ^ sender, VisibilityChangedEventArgs ^ args) {
        App._minimized = !args->Visible;
    }
    void OnWindowClosed(CoreWindow ^ sender, CoreWindowEventArgs ^ args) { App._close = true; } // this is not called, but do this just in case, as the tutorial does
    void OnWindowActivated(CoreWindow ^ sender, WindowActivatedEventArgs ^ args) {
        if (App._closed)
            return; // do nothing if app called 'Exit'
        App.setActive(args->WindowActivationState != CoreWindowActivationState::Deactivated);
    }
    void OnDpiChanged(DisplayInformation ^ sender, Object ^ args) {
        ScreenScale = sender->RawPixelsPerViewPixel;
        setMode();
        D.aspectRatioEx(false); // bug workaround: on Windows when app is started on a secondary monitor, then initially SwapChain output will point to the main monitor output, which will make the aspect calculation incorrect if app starts fullscreen, however it was noticed that 'OnDpiChanged' will be called soon after that, and at this point, output will be correct, so reset aspect ratio here with correct output
    }
    void OnOrientationChanged(DisplayInformation ^ sender, Object ^ args) {
        setOrientation(sender->CurrentOrientation, sender->NativeOrientation);
        setMode();
    }
    void OnColorProfileChanged(DisplayInformation ^ sender, Object ^ args) {
        D.getScreenInfo();
        D.setColorLUT();
    }
    void OnAdvancedColorInfoChanged(DisplayInformation ^ sender, Object ^ args) {
        D.getScreenInfo();
        D.setColorLUT();
    }
    void OnDisplayContentsInvalidated(DisplayInformation ^ sender, Object ^ args) {
        /*// The D3D Device is no longer valid if the default adapter changed since the device
          // was created or if the device has been removed.

          // First, get the information for the default adapter from when the device was created.
          ComPtr<IDXGIDevice3> dxgiDevice;
          DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

          ComPtr<IDXGIAdapter> deviceAdapter;
          DX::ThrowIfFailed(dxgiDevice->GetAdapter(&deviceAdapter));

          ComPtr<IDXGIFactory2> deviceFactory;
          DX::ThrowIfFailed(deviceAdapter->GetParent(IID_PPV_ARGS(&deviceFactory)));

          ComPtr<IDXGIAdapter1> previousDefaultAdapter;
          DX::ThrowIfFailed(deviceFactory->EnumAdapters1(0, &previousDefaultAdapter));

          DXGI_ADAPTER_DESC previousDesc;
          DX::ThrowIfFailed(previousDefaultAdapter->GetDesc(&previousDesc));

          // Next, get the information for the current default adapter.

          ComPtr<IDXGIFactory2> currentFactory;
          DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&currentFactory)));

          ComPtr<IDXGIAdapter1> currentDefaultAdapter;
          DX::ThrowIfFailed(currentFactory->EnumAdapters1(0, &currentDefaultAdapter));

          DXGI_ADAPTER_DESC currentDesc;
          DX::ThrowIfFailed(currentDefaultAdapter->GetDesc(&currentDesc));

          // If the adapter LUIDs don't match, or if the device reports that it has been removed,
          // a new D3D device must be created.

          if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
          previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
          FAILED(m_d3dDevice->GetDeviceRemovedReason()))
          {
             // Release references to resources related to the old device.
             dxgiDevice = nullptr;
             deviceAdapter = nullptr;
             deviceFactory = nullptr;
             previousDefaultAdapter = nullptr;

             // Create a new device and swap chain.
             HandleDeviceLost();
          }*/
    }
    void OnMouseMoved(MouseDevice ^ mouseDevice, MouseEventArgs ^ args) // this is not clipped by desktop (if mouse is moved but the cursor remains in the same position, it will still generate move events). WINDOWS_NEW BUG: this will not be called if mouse hovers over title bar or border, but if mouse is moved quickly outside without touching the borders, then it will continue to receive data
    {
        Ms._delta_rel.x += args->MouseDelta.X;
        Ms._delta_rel.y -= args->MouseDelta.Y;
    }
    void OnPointerMoved(CoreWindow ^ sender, PointerEventArgs ^ args) {
        PointerPoint ^ pointer = args->CurrentPoint;
        switch (pointer->PointerDevice->PointerDeviceType) {
        case PointerDeviceType::Mouse: {
            // position is handled in 'Ms.update' because this is not called when mouse is outside the window
            UpdateMouseButton(pointer->Properties); // need to update buttons, because this is the only place where button states are reported if there already other buttons pressed
        } break;

        default: // pen, touch
        {
            if (Touch *touch = FindTouchByHandle(CPtr(pointer->PointerId))) {
                VecI2 pixeli(DipsToPixelsI(pointer->Position.X), DipsToPixelsI(pointer->Position.Y));
                touch->_delta_pixeli_clp += pixeli - touch->_pixeli;
                touch->_pixeli = pixeli;
                touch->_pos = D.windowPixelToScreen(pixeli);
            }
        } break;
        }
    }
    void OnPointerEntered(CoreWindow ^ sender, PointerEventArgs ^ args) {
        PointerPoint ^ pointer = args->CurrentPoint;
        switch (pointer->PointerDevice->PointerDeviceType) {
        case PointerDeviceType::Mouse: {
            Ms._on_client = true;
            // Ms._delta_pixeli_clp=Ms._window_pixeli-pixeli; don't calculate delta here to avoid big jumps
            // Ms._window_pixeli=pixeli; this is handled in 'Ms.update'
        } break;

        default: // pen, touch
        {
            CPtr id = CPtr(pointer->PointerId);
            VecI2 pixeli(DipsToPixelsI(pointer->Position.X), DipsToPixelsI(pointer->Position.Y));
            Vec2 pos = D.windowPixelToScreen(pixeli);
            Touch *touch = FindTouchByHandle(id);
            if (!touch)
                touch = &Touches.New().init(pixeli, pos, id, pointer->PointerDevice->PointerDeviceType == PointerDeviceType::Pen);
            else {
                touch->_remove = false; // disable 'remove' in case it was enabled (for example the same touch exited in same/previous frame)
                touch->_pixeli = pixeli;
                touch->_pos = pos;
            }
        } break;
        }
    }
    void OnPointerExited(CoreWindow ^ sender, PointerEventArgs ^ args) {
        PointerPoint ^ pointer = args->CurrentPoint;
        switch (pointer->PointerDevice->PointerDeviceType) {
        case PointerDeviceType::Mouse: {
            Ms._on_client = false;
            // Ms._delta_pixeli_clp=Ms._window_pixeli-pixeli; this is handled in 'Ms.update'
            // Ms._window_pixeli=pixeli; this is handled in 'Ms.update'
        } break;

        default: // pen, touch
        {
            if (Touch *touch = FindTouchByHandle(CPtr(pointer->PointerId))) {
                VecI2 pixeli(DipsToPixelsI(pointer->Position.X), DipsToPixelsI(pointer->Position.Y));
                touch->_delta_pixeli_clp += pixeli - touch->_pixeli;
                touch->_pixeli = pixeli;
                touch->_pos = D.windowPixelToScreen(pixeli);
                touch->_remove = true;
                if (touch->_state & BS_ON) // check for state in case it was manually eaten
                {
                    touch->_state |= BS_RELEASED;
                    touch->_state &= ~BS_ON;
                }
            }
        } break;
        }
    }
    void OnPointerPressed(CoreWindow ^ sender, PointerEventArgs ^ args) {
        PointerPoint ^ pointer = args->CurrentPoint;
        switch (pointer->PointerDevice->PointerDeviceType) {
        case PointerDeviceType::Mouse: {
            UpdateMouseButton(pointer->Properties); // this will get called only if no other mouse button is currently pressed, for simultaneous presses 'OnPointerMoved' has to be checked
        } break;

        default: // pen, touch
        {
            if (Touch *touch = FindTouchByHandle(CPtr(pointer->PointerId))) {
                touch->_state = BS_ON | BS_PUSHED;
            }
        } break;
        }
    }
    void OnPointerReleased(CoreWindow ^ sender, PointerEventArgs ^ args) {
        PointerPoint ^ pointer = args->CurrentPoint;
        switch (pointer->PointerDevice->PointerDeviceType) {
        case PointerDeviceType::Mouse: {
            UpdateMouseButton(pointer->Properties); // this will get called only after all mouse buttons have been released, for individual button releases 'OnPointerMoved' has to be checked
        } break;

        default: // pen, touch
        {
            if (Touch *touch = FindTouchByHandle(CPtr(pointer->PointerId)))
                if (touch->_state & BS_ON) // check for state in case it was manually eaten
                {
                    touch->_state |= BS_RELEASED;
                    touch->_state &= ~BS_ON;
                }
        } break;
        }
    }
    void OnPointerWheelChanged(CoreWindow ^ sender, PointerEventArgs ^ args) {
        PointerPoint ^ pointer = args->CurrentPoint;
        switch (pointer->PointerDevice->PointerDeviceType) {
        case PointerDeviceType::Mouse: {
            if (pointer->Properties->IsHorizontalMouseWheel)
                Ms._wheel.x += Flt(pointer->Properties->MouseWheelDelta) / WHEEL_DELTA;
            else
                Ms._wheel.y += Flt(pointer->Properties->MouseWheelDelta) / WHEEL_DELTA;
        } break;
        }
    }
    void OnBackRequested(Object ^ sender, BackRequestedEventArgs ^ args) // This is called when pressing Gamepad B button, and on Windows Phone onscreen back button in the nav bar. By default these actions suspend the game
    {
        args->Handled = true; // !! THIS HAS TO BE CALLED TO DISABLE APP CLOSE !!
                              /*Don't trigger this because this function is also called due to Gamepad B buttons
                                Kb.push   (KB_NAV_BACK, -1);
                                Kb.release(KB_NAV_BACK); // release immediately because there's no callback for a release*/
    }
#if KEY_EVENTS
    void OnAcceleratorKeyActivated(CoreDispatcher ^ sender, AcceleratorKeyEventArgs ^ args) {
        // !! Warning: On International keyboards RightAlt (AltGr) also triggers LeftCtrl, there's no way to workaround this, LeftCtrl is pressed down, and notification of RightAlt can happen even a few frames later !!
        Int scan_code = args->KeyStatus.ScanCode;
#if DEBUG && 0
#pragma message("!! Warning: Use this only for debugging !!")
        Str s;
        switch (Windows::UI::Core::CoreAcceleratorKeyEventType type = args->EventType) {
        case Windows::UI::Core::CoreAcceleratorKeyEventType::Character:
            s = S + "Char " + Char(args->VirtualKey) + ' ';
            break;
        case Windows::UI::Core::CoreAcceleratorKeyEventType::SystemCharacter:
            s = S + "SysChar " + Char(args->VirtualKey) + ' ';
            break;
        case Windows::UI::Core::CoreAcceleratorKeyEventType::UnicodeCharacter:
            s = "UniChar";
            break;
        case Windows::UI::Core::CoreAcceleratorKeyEventType::KeyDown:
            s = "KeyDown";
            break;
        case Windows::UI::Core::CoreAcceleratorKeyEventType::KeyUp:
            s = "KeyUp";
            break;
        case Windows::UI::Core::CoreAcceleratorKeyEventType::SystemKeyDown:
            s = "SysKeyDown";
            break;
        case Windows::UI::Core::CoreAcceleratorKeyEventType::SystemKeyUp:
            s = "SysKeyUp";
            break;
        case Windows::UI::Core::CoreAcceleratorKeyEventType::DeadCharacter:
            s = "DeadChar";
            break;
        case Windows::UI::Core::CoreAcceleratorKeyEventType::SystemDeadCharacter:
            s = "SysDeadChar";
            break;
        }
        KB_KEY key = KB_KEY(args->VirtualKey);
        LogN(S + "frame:" + Time.frame() + ", " + s + " scan_code:" + scan_code + ' ' + key + '(' + Kb.keyName(key) + "), ext:" + args->KeyStatus.IsExtendedKey + ", released:" + args->KeyStatus.IsKeyReleased + ", wasDown:" + args->KeyStatus.WasKeyDown + ", repeat:" + args->KeyStatus.RepeatCount);
#endif
        switch (args->EventType) {
        case Windows::UI::Core::CoreAcceleratorKeyEventType::KeyDown:
        case Windows::UI::Core::CoreAcceleratorKeyEventType::SystemKeyDown: {
            KB_KEY key = KB_KEY(args->VirtualKey);
            switch (key) {
            case KB_CTRL:
                key = (args->KeyStatus.IsExtendedKey ? KB_RCTRL : KB_LCTRL);
                break;
            case KB_SHIFT:
                key = ((scan_code == 42) ? KB_LSHIFT : KB_RSHIFT);
                break; // 42=KB_LSHIFT, 54=KB_RSHIFT
            case KB_ALT:
                key = (args->KeyStatus.IsExtendedKey ? KB_RALT : KB_LALT);
                break;
            }
            if (App._waiting)
                Events.New().keyDown(key, scan_code);
            else
                Kb.push(key, scan_code); // if app is in special loop, then we need to record events, and execute them later
        } break;

        case Windows::UI::Core::CoreAcceleratorKeyEventType::Character:
        case Windows::UI::Core::CoreAcceleratorKeyEventType::SystemCharacter: {
            Char c = (Char)args->VirtualKey;
            if (App._waiting)
                Events.New().chr(c, scan_code);
            else
                Kb.queue(c, scan_code); // if app is in special loop, then we need to record events, and execute them later
        } break;

        case Windows::UI::Core::CoreAcceleratorKeyEventType::KeyUp:
        case Windows::UI::Core::CoreAcceleratorKeyEventType::SystemKeyUp: {
            KB_KEY key = KB_KEY(args->VirtualKey);
            switch (key) {
            case KB_CTRL:
                key = (args->KeyStatus.IsExtendedKey ? KB_RCTRL : KB_LCTRL);
                break;
            case KB_SHIFT:
                key = ((scan_code == 42) ? KB_LSHIFT : KB_RSHIFT);
                break; // 42=KB_LSHIFT, 54=KB_RSHIFT
            case KB_ALT:
                key = (args->KeyStatus.IsExtendedKey ? KB_RALT : KB_LALT);
                break;
            }
            if (App._waiting)
                Events.New().keyUp(key);
            else
                Kb.release(key); // if app is in special loop, then we need to record events, and execute them later
        } break;
        }
    }
#else
    void OnKeyDown(CoreWindow ^ sender, KeyEventArgs ^ args) // mouse buttons are not passed here
    {
        Int scan_code = args->KeyStatus.ScanCode;
        KB_KEY key = KB_KEY(args->VirtualKey);
        // LogN(S+"OnKeyDown: scan_code:"+scan_code+' '+key+'('+Kb.keyName(key)+"), ext:"+args->KeyStatus.IsExtendedKey+", released:"+args->KeyStatus.IsKeyReleased+", wasDown:"+args->KeyStatus.WasKeyDown+", repeat:"+args->KeyStatus.RepeatCount);
        switch (key) {
        case KB_CTRL:
            key = (args->KeyStatus.IsExtendedKey ? KB_RCTRL : KB_LCTRL);
            break;
        case KB_SHIFT:
            key = ((scan_code == 42) ? KB_LSHIFT : KB_RSHIFT);
            break; // 42=KB_LSHIFT, 54=KB_RSHIFT
        case KB_ALT:
            key = (args->KeyStatus.IsExtendedKey ? KB_RALT : KB_LALT);
            break;
        }
        if (App._waiting)
            Events.New().keyDown(key, scan_code);
        else
            Kb_push(key, scan_code); // if app is in special loop, then we need to record events, and execute them later
    }
    void OnCharacterReceived(CoreWindow ^ sender, CharacterReceivedEventArgs ^ args) {
        Char c = Char(args->KeyCode);
        Int scan_code = args->KeyStatus.ScanCode;
        if (App._waiting)
            Events.New().chr(c, scan_code);
        else
            Kb.queue(c, scan_code); // if app is in special loop, then we need to record events, and execute them later
    }
    void OnKeyUp(CoreWindow ^ sender, KeyEventArgs ^ args) // mouse buttons are not passed here
    {
        Int scan_code = args->KeyStatus.ScanCode;
        KB_KEY key = KB_KEY(args->VirtualKey);
        // LogN(S+"OnKeyUp: scan_code:"+(args->KeyStatus.ScanCode)+' '+key+'('+Kb.keyName(key)+"), ext:"+args->KeyStatus.IsExtendedKey+", released:"+args->KeyStatus.IsKeyReleased+", wasDown:"+args->KeyStatus.WasKeyDown+", repeat:"+args->KeyStatus.RepeatCount);
        switch (key) {
        case KB_CTRL:
            key = (args->KeyStatus.IsExtendedKey ? KB_RCTRL : KB_LCTRL);
            break;
        case KB_SHIFT:
            key = ((scan_code == 42) ? KB_LSHIFT : KB_RSHIFT);
            break; // 42=KB_LSHIFT, 54=KB_RSHIFT
        case KB_ALT:
            key = (args->KeyStatus.IsExtendedKey ? KB_RALT : KB_LALT);
            break;
        }
        if (App._waiting)
            Events.New().keyUp(key);
        else
            Kb.release(key); // if app is in special loop, then we need to record events, and execute them later
    }
#endif

#if JP_GAMEPAD_INPUT
    // !! THESE ARE CALLED ON SECONDARY THREADS !!
    void OnGamepadAdded(Object ^ sender, Gamepad ^ gamepad) { GamepadChanged(true, gamepad, null); }
    void OnGamepadRemoved(Object ^ sender, Gamepad ^ gamepad) { GamepadChanged(false, gamepad, null); }

    void OnRawGameControllerAdded(Object ^ sender, RawGameController ^ raw_game_controller) { GamepadChanged(true, null, raw_game_controller); }
    void OnRawGameControllerRemoved(Object ^ sender, RawGameController ^ raw_game_controller) { GamepadChanged(false, null, raw_game_controller); }

    void OnGamepadUserChanged(Windows::Gaming::Input::IGameController ^ controller, Windows::System::UserChangedEventArgs ^ args) {
        if (auto joypad_user_changed = App.joypad_user_changed) // copy to temporary first, to avoid multi-thread issues
        {
            Int joypad_i = FindJoypadI(controller);
            if (joypad_i >= 0)
                joypad_user_changed(Joypads[joypad_i].id());
        }
    }
#elif JP_X_INPUT
    void OnGamepadRemoved(Object ^ sender, Gamepad ^ gamepad) { App.addFuncCall(ListJoypads); } // can't call 'ListJoypads' directly because at this stage the XInput state might still be old, and return results from previous frame while the Joypad is still   available (this happened during testing)
    void OnGamepadAdded(Object ^ sender, Gamepad ^ gamepad) { App.addFuncCall(ListJoypads); }   // can't call 'ListJoypads' directly because at this stage the XInput state might still be old, and return results from previous frame while the Joypad is still unavailable (this happened during testing)
#endif

    void OnAccelerometerChanged(Sensors::Accelerometer ^ accelerometer, AccelerometerReadingChangedEventArgs ^ args) {
        AccelerometerValue.set(args->Reading->AccelerationX, args->Reading->AccelerationY, -args->Reading->AccelerationZ) *= 9.80665f;
    }
    void OnGyrometerChanged(Sensors::Gyrometer ^ Gyrometer, GyrometerReadingChangedEventArgs ^ args) {
        GyroscopeValue.set(args->Reading->AngularVelocityX, args->Reading->AngularVelocityY, -args->Reading->AngularVelocityZ);
    }
    void OnMagnetometerChanged(Sensors::Magnetometer ^ Magnetometer, MagnetometerReadingChangedEventArgs ^ args) {
        MagnetometerValue.set(args->Reading->MagneticFieldX, args->Reading->MagneticFieldY, -args->Reading->MagneticFieldZ);
    }

    // Input
    void OnInputPaneHiding(InputPane ^ sender, InputPaneVisibilityEventArgs ^ args) {
        Kb._visible = false;
        Kb.screenChanged();
    }
    void OnInputPaneShowing(InputPane ^ sender, InputPaneVisibilityEventArgs ^ args) {
        Kb._visible = true;
        Kb._recti.setLD(DipsToPixelsI(sender->OccludedRect.X), DipsToPixelsI(sender->OccludedRect.Y), DipsToPixelsI(sender->OccludedRect.Width), DipsToPixelsI(sender->OccludedRect.Height));
        Kb.screenChanged();
    }

    void OnInputFocusRemoved(Windows::UI::Text::Core::CoreTextEditContext ^ sender, Platform::Object ^ args) {
        if (auto input_pane = Windows::UI::ViewManagement::InputPane::GetForCurrentView())
            input_pane->TryHide();
    }
    void OnInputTextRequested(Windows::UI::Text::Core::CoreTextEditContext ^ sender, Windows::UI::Text::Core::CoreTextTextRequestedEventArgs ^ args) {
        ScreenKeyboard sk;
        sk.set();
        if (sk.text)
            if (auto request = args->Request)
                request->Text = ref new Platform::String(Trim(*sk.text, request->Range.StartCaretPosition, request->Range.EndCaretPosition - request->Range.StartCaretPosition));
    }
    void OnInputSelectionRequested(Windows::UI::Text::Core::CoreTextEditContext ^ sender, Windows::UI::Text::Core::CoreTextSelectionRequestedEventArgs ^ args) {
        ScreenKeyboard sk;
        sk.set();
        Windows::UI::Text::Core::CoreTextRange selection;
        MinMax(sk.start, sk.end, selection.StartCaretPosition, selection.EndCaretPosition);
        args->Request->Selection = selection;
    }
    void OnInputTextUpdating(Windows::UI::Text::Core::CoreTextEditContext ^ sender, Windows::UI::Text::Core::CoreTextTextUpdatingEventArgs ^ args) {
        /* already handled by 'OnAcceleratorKeyActivated/OnCharacterReceived'
        ScreenKeyboard sk; sk.set();
        Str text;
        if(sk.text)text=Trim(*sk.text, 0, args->Range.StartCaretPosition)+args->Text->Data()+Trim(*sk.text, args->Range.EndCaretPosition, INT_MAX);
        else       text=                                                  args->Text->Data();
        ScreenKeyboard::Set(text);
        ScreenKeyboard::Set(args->NewSelection.EndCaretPosition, args->NewSelection.StartCaretPosition);*/
    }
    void OnInputSelectionUpdating(Windows::UI::Text::Core::CoreTextEditContext ^ sender, Windows::UI::Text::Core::CoreTextSelectionUpdatingEventArgs ^ args) {
        /* already handled by 'OnAcceleratorKeyActivated/OnCharacterReceived'
        ScreenKeyboard::Set(args->Selection.EndCaretPosition, args->Selection.StartCaretPosition);*/
    }
    /*void OnInputLayoutRequested(Windows::UI::Text::Core::CoreTextEditContext ^sender, Windows::UI::Text::Core::CoreTextLayoutRequestedEventArgs ^args)
    {
       CoreTextLayoutRequest request = args.Request;

       // Get the screen coordinates of the entire control and the selected text.
       // This information is used to position the IME candidate window.

       // First, get the coordinates of the edit control and the selection
       // relative to the Window.
       Rect contentRect = GetElementRect(ContentPanel);
       Rect selectionRect = GetElementRect(SelectionText);

       // Next, convert to screen coordinates in view pixels.
       Rect windowBounds = Window.Current.CoreWindow.Bounds;
       contentRect.X += windowBounds.X;
       contentRect.Y += windowBounds.Y;
       selectionRect.X += windowBounds.X;
       selectionRect.Y += windowBounds.Y;

       // Finally, scale up to raw pixels.
       double scaleFactor = DisplayInformation.GetForCurrentView().RawPixelsPerViewPixel;

       contentRect = ScaleRect(contentRect, scaleFactor);
       selectionRect = ScaleRect(selectionRect, scaleFactor);

       // This is the bounds of the selection.
       // Note: If you return bounds with 0 width and 0 height, candidates will not appear while typing.
       request.LayoutBounds.TextBounds = selectionRect;

       //This is the bounds of the whole control
       request.LayoutBounds.ControlBounds = contentRect;
    }*/
    void OnDeviceAdded(Windows::Devices::Enumeration::DeviceWatcher ^ watcher, Windows::Devices::Enumeration::DeviceInformation ^ device) { App.includeFuncCall(DeviceChanged); }
    void OnDeviceUpdate(Windows::Devices::Enumeration::DeviceWatcher ^ watcher, Windows::Devices::Enumeration::DeviceInformationUpdate ^ device_update) { App.includeFuncCall(DeviceChanged); }

    // custom methods
    void setOrientation(DisplayOrientations orientation, DisplayOrientations native_orientation) {
        DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_IDENTITY;
        switch (native_orientation) // this can only be Landscape or Portrait even though the DisplayOrientations enum has other values
        {
        case DisplayOrientations::Landscape:
            switch (orientation) {
                // case DisplayOrientations::Landscape       : rotation=DXGI_MODE_ROTATION_IDENTITY ; break;
            case DisplayOrientations::Portrait:
                rotation = DXGI_MODE_ROTATION_ROTATE270;
                break;
            case DisplayOrientations::LandscapeFlipped:
                rotation = DXGI_MODE_ROTATION_ROTATE180;
                break;
            case DisplayOrientations::PortraitFlipped:
                rotation = DXGI_MODE_ROTATION_ROTATE90;
                break;
            }
            break;

        case DisplayOrientations::Portrait:
            switch (orientation) {
            case DisplayOrientations::Landscape:
                rotation = DXGI_MODE_ROTATION_ROTATE90;
                break;
                // case DisplayOrientations::Portrait        : rotation=DXGI_MODE_ROTATION_IDENTITY ; break;
            case DisplayOrientations::LandscapeFlipped:
                rotation = DXGI_MODE_ROTATION_ROTATE270;
                break;
            case DisplayOrientations::PortraitFlipped:
                rotation = DXGI_MODE_ROTATION_ROTATE180;
                break;
            }
            break;
        }

#if 0 // set based on absolute pose (this will return landscape for laptops)
      switch(orientation)
      {
         default                                   : App._orientation=DIR_UP   ; break; // DisplayOrientations::Portrait
         case DisplayOrientations::PortraitFlipped : App._orientation=DIR_DOWN ; break;
         case DisplayOrientations::Landscape       : App._orientation=DIR_LEFT ; break;
         case DisplayOrientations::LandscapeFlipped: App._orientation=DIR_RIGHT; break;
      }
#else // set based on relative rotation (this will return portrait for laptops)
        switch (rotation) {
        case DXGI_MODE_ROTATION_IDENTITY:
            App._orientation = DIR_UP;
            break;
        case DXGI_MODE_ROTATION_ROTATE180:
            App._orientation = DIR_DOWN;
            break;
        case DXGI_MODE_ROTATION_ROTATE90:
            App._orientation = DIR_LEFT;
            break;
        case DXGI_MODE_ROTATION_ROTATE270:
            App._orientation = DIR_RIGHT;
            break;
        }
#endif
    }
    void setMode() {
        if (App._closed)
            return; // do nothing if app called 'Exit'
        VecI2 mode(DipsToPixelsI(App.window()->Bounds.Width), DipsToPixelsI(App.window()->Bounds.Height));
        D.modeSet(mode.x, mode.y, -1);
        D.getScreenInfo();
        D.setColorLUT();
    }
} static ^ FrameworkViewObj;
ref struct FrameworkViewSource sealed : IFrameworkViewSource {
    virtual IFrameworkView ^ CreateView() {
        return FrameworkViewObj = ref new FrameworkView();
    }
};
#if JP_GAMEPAD_INPUT
void GamepadChange::process() {
    if (added) {
        if (gamepad && FindJoypadI(gamepad) >= 0 || raw_game_controller && FindJoypadI(raw_game_controller) >= 0)
            return; // make sure it's not already listed

        UInt joypad_id;
        if (gamepad) // gamepad callback
        {
            raw_game_controller = Windows::Gaming::Input::RawGameController::FromGameController(gamepad);
        } else // raw_game_controller callback
        {
            if (Windows::Gaming::Input::Gamepad::FromGameController(raw_game_controller))
                return; // if this is a gamepad, then ignore this 'raw_game_controller' callback, as it will be processed using 'gamepad' callback, to avoid having the same gamepad listed twice
        }

        if (raw_game_controller) {
            C wchar_t *controller_id = raw_game_controller->NonRoamableId->Data();
            joypad_id = xxHash64_32Mem(controller_id, SIZE(*controller_id) * Length(controller_id));
        } else
            joypad_id = 0;
        joypad_id = NewJoypadID(joypad_id); // make sure it's not used yet !! set this before creating new 'Joypad' !!
        SyncLocker lock(JoypadLock);
        Bool added;
        Joypad &joypad = GetJoypad(joypad_id, added);
        if (added) {
            joypad._gamepad = gamepad;
            joypad._raw_game_controller = raw_game_controller;
            if (raw_game_controller) {
                joypad._name = raw_game_controller->DisplayName->Data();
                joypad._buttons = Min(raw_game_controller->ButtonCount, Elms(joypad._remap));
                joypad._switches = raw_game_controller->SwitchCount;
                joypad._axes = raw_game_controller->AxisCount;

                if (auto motors = raw_game_controller->ForceFeedbackMotors)
                    joypad._vibrations = (motors->Size > 0);
                joypad.setInfo(raw_game_controller->HardwareVendorId, raw_game_controller->HardwareProductId);

                if (!joypad._gamepad)
                    REPA(joypad._state) // this is needed only for 'raw_game_controller', check after 'setInfo' because that might delete '_gamepad'
                    {
                        auto &state = joypad._state[i]; // allocations below also zero memory, which is what we need
                        state.button = ref new Platform::Array<bool>(joypad._buttons);
                        state.Switch = ref new Platform::Array<GameControllerSwitchPosition>(joypad._switches);
                        state.axis = ref new Platform::Array<double>(joypad._axes);
                        REP(Min(joypad._axes, 4))
                        state.axis->Data[i] = 0.5f; // set initial state to centered
                    }
            }
            // set callback after everything was set, in case it's called right away
            if (joypad._gamepad)
                joypad._gamepad->UserChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Gaming::Input::IGameController ^, Windows::System::UserChangedEventArgs ^>(FrameworkViewObj, &FrameworkView::OnGamepadUserChanged);
            else if (joypad._raw_game_controller)
                joypad._raw_game_controller->UserChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Gaming::Input::IGameController ^, Windows::System::UserChangedEventArgs ^>(FrameworkViewObj, &FrameworkView::OnGamepadUserChanged);

            if (auto func = App.joypad_changed)
                func(); // call at the end
        }
    } else {
        if (gamepad)
            Joypads.remove(FindJoypadI(gamepad));
        if (raw_game_controller)
            Joypads.remove(FindJoypadI(raw_game_controller));
    }
}
#endif
void Application::wait(SyncEvent &event) {
    if (mainThread()) // on the main thread we need to keep processing events
    {
        _waiting = true; // specify that we're inside this special waiting loop, this is needed so if any events occur during callback processing, then we will record them and execute them at a later time, for example this is done so no key pushes are detected at this stage (which can occur in State Update or Draw) but they will be detected once Update and Draw are finished
        for (;;)         // start loop
        {
            Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent); // this may call our callbacks, 'ProcessOneAndAllPending'= can't be used because apparently tasks don't count as events, and this will wait until some other events occur, even though we have tasks waiting. We would have to use 'PostEvent', however in tests results were slightly slower, so don't use.
            if (event.wait(1))
                break; // wait after processing all events queued up so far to make sure we don't stall the events (in case this method is called a lot of times, for example 1000 times with 'event.wait' before processing events in worst case would prevent events from being processed for 1 second)
        }
        _waiting = false;
    } else
        event.wait();
}
#endif
/******************************************************************************/
} // namespace EE
/******************************************************************************/

#if WINDOWS_OLD
INT WINAPI wWinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    InitThreadedLoggerForCPP();
    char exePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    if (!elevatePrivileges(exePath)) {
        GlobalsLoggerInstance::LoggerInstance
            .logMessageAsync(LogLevel::INFO, __FILE__, __LINE__, "Failed to get admin privileges");
        GlobalsLoggerInstance::LoggerInstance.ExitLoggerThread();
        return EXIT_FAILURE;
    }
    if (strcmp(exePath, "Titan Editor.exe") == 0) {
        LONG result = DisableMemoryCrashDump();
        if (result == ERROR_SUCCESS) {
            GlobalsLoggerInstance::LoggerInstance.logMessageAsync(
                LogLevel::INFO, __FILE__, __LINE__,
                "Init Logger Thread in wWinMain method");
        } else {
            GlobalsLoggerInstance::LoggerInstance.logMessageAsync(
                LogLevel::INFO, __FILE__, __LINE__,
                "Cannot Create Reg values to disable Memory Crash Dump");
            GlobalsLoggerInstance::LoggerInstance.ExitLoggerThread();
        }
    }

#if JP_GAMEPAD_INPUT
    RoInitialize(RO_INIT_MULTITHREADED);
#endif

    const Int start = 1; // start from #1, because #0 is always the executable name (if file name has spaces, then they're included in the #0 argv)
    App.cmd_line.setNum(__argc - start);
    FREPAO(App.cmd_line) = __wargv[start + i];

    App._hinstance = hinstance;
    if (App.create())
        App.loop();
    App.del();

    // Exit Logger Thread And Save logs to file
    GlobalsLoggerInstance::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__,
        "Stop Logger Thread + Game exited in wWinMain method");

    return 0;
}

#elif WINDOWS_NEW
[Platform::MTAThread] int main(Platform::Array<Platform::String ^> ^ args) {
    InitThreadedLoggerForCPP();
    char exePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    if (!elevatePrivileges(exePath)) {
        GlobalsLoggerInstance::LoggerInstance
            .logMessageAsync(LogLevel::INFO, __FILE__, __LINE__, "Failed to get admin privileges");
        GlobalsLoggerInstance::LoggerInstance.ExitLoggerThread();
        return EXIT_FAILURE;
    }
    // Appel à DisableMemoryCrashDump seulement si le nom de fichier est "Titan Editor.exe"
    if (strcmp(exePath, "Titan Editor.exe") == 0) {
        LONG result = DisableMemoryCrashDump();
        if (result == ERROR_SUCCESS) {
            GlobalsLoggerInstance::LoggerInstance.logMessageAsync(
                LogLevel::INFO, __FILE__, __LINE__,
                "Init Logger Thread in main method");
        } else {
            GlobalsLoggerInstance::LoggerInstance.logMessageAsync(
                LogLevel::INFO, __FILE__, __LINE__,
                "Cannot Create Reg values to disable Memory Crash Dump");
            GlobalsLoggerInstance::LoggerInstance.ExitLoggerThread();
        }
    }

    /* TODO: WINDOWS_NEW setting initial window size and fullscreen mode - check in the future
          changing these didn't make any difference at this launch, only next launch got affected
          if(1)ApplicationView::PreferredLaunchViewSize = Size(300, 300);
          else ApplicationView::PreferredLaunchViewSize = Size(600, 600);
          ApplicationView::PreferredLaunchWindowingMode = ApplicationViewWindowingMode::PreferredLaunchViewSize;
        //ApplicationView::PreferredLaunchWindowingMode = ApplicationViewWindowingMode::Auto;
        //ApplicationView::PreferredLaunchWindowingMode = ApplicationViewWindowingMode::FullScreen;*/

    if (args) {
        const Int start = 1; // start from #1, because #0 is always the executable name (if file name has spaces, then they're included in the #0 argv)
        App.cmd_line.setNum(args->Length - start);
        FREPAO(App.cmd_line) = args->get(start + i)->Data();
    }

    Windows::ApplicationModel::Core::CoreApplication::Run(ref new FrameworkViewSource());

    // Exit Logger Thread And Save logs to file
    GlobalsLoggerInstance::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__,
        "Stop Logger Thread + Game exited in main method");

    return 0;
}
#endif
/******************************************************************************/
