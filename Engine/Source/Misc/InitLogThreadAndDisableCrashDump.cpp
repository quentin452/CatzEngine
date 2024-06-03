#include "../../stdafx.h"
std::unique_ptr<LoggerThread> LoggerThread::LoggerInstanceT;
std::wstring InitThreadAndDisableDump::ConvertToWideString(const std::string &narrow_str) {
    if (narrow_str.empty())
        return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &narrow_str[0], (int)narrow_str.size(), NULL, 0);
    std::wstring wide_str(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &narrow_str[0], (int)narrow_str.size(), &wide_str[0], size_needed);
    return wide_str;
}

// Avoid making 70 mb of crash dump "every time" you close the app (avoid https://github.com/quentin452/CatzEngine/issues/24 + https://github.com/quentin452/CatzEngine/issues/26)
long InitThreadAndDisableDump::DisableMemoryCrashDump(std::string _exe_name) {
    HKEY hKey1 = nullptr;
    LONG result;
    DWORD dwDisposition;
    DWORD CustomDumpFlags = 0x41000200;
    DWORD DumpCount = 0;
    DWORD DumpType = 0;

    std::wstring wide_game_exe = ConvertToWideString(_exe_name + ".exe");
    std::wstring registryPath1 = L"SOFTWARE\\Microsoft\\Windows\\Windows Error Reporting\\LocalDumps\\" + wide_game_exe;
    result = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        registryPath1.c_str(),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey1,
        &dwDisposition);

    if (result != ERROR_SUCCESS) {
        return result;
    }

    result = RegSetValueEx(hKey1, L"CustomDumpFlags", 0, REG_DWORD, (BYTE *)&CustomDumpFlags, sizeof(DWORD));
    if (result != ERROR_SUCCESS) {
        RegCloseKey(hKey1);
        return result;
    }

    result = RegSetValueEx(hKey1, L"DumpCount", 0, REG_DWORD, (BYTE *)&DumpCount, sizeof(DWORD));
    if (result != ERROR_SUCCESS) {
        RegCloseKey(hKey1);
        return result;
    }

    result = RegSetValueEx(hKey1, L"DumpType", 0, REG_DWORD, (BYTE *)&DumpType, sizeof(DWORD));
    if (result != ERROR_SUCCESS) {
        RegCloseKey(hKey1);
        return result;
    }

    RegCloseKey(hKey1);
    return result;
}
bool InitThreadAndDisableDump::elevatePrivileges(const char *exePath) {
    bool isElevated = false;
    HANDLE hToken = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION Elevation;
        DWORD dwSize;
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &dwSize)) {
            isElevated = Elevation.TokenIsElevated;
        }
        CloseHandle(hToken);
    }

    if (!isElevated) {
        int msgboxID = MessageBox(
            NULL,
            (LPCWSTR)L"This program requires administrator privileges. Please close and restart the program as administrator.",
            (LPCWSTR)L"Request for administrator privileges",
            MB_ICONWARNING | MB_OK);
        exit(0);
    }

    return isElevated;
}

bool InitThreadAndDisableDump::GetExeNameAndCheckAdminPrivileges(char *exePath, char *exeName, bool *hasAdminPrivileges) {
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    _splitpath_s(exePath, NULL, 0, NULL, 0, exeName, MAX_PATH, NULL, 0);

    *hasAdminPrivileges = InitThreadAndDisableDump::elevatePrivileges(exePath);
    if (!*hasAdminPrivileges) {
        LoggerThread::GetLoggerThread().logMessageAsync(
            LogLevel::INFO, __FILE__, __LINE__, "Failed to get admin privileges");
        return false;
    }
    return true;
}
bool InitThreadAndDisableDump::Init_elevatePrivile_DisableMemoryCrashDump_For_Games(std::string &_exe_game) {
    char exePath[MAX_PATH];
    char exeName[MAX_PATH];
    bool hasAdminPrivileges;
    if (strcmp(exeName, _exe_game.c_str()) == 0) {
        if (!InitThreadAndDisableDump::GetExeNameAndCheckAdminPrivileges(exePath, exeName, &hasAdminPrivileges)) {
            return false;
        }
        InitThreadAndDisableDump::DisableMemoryCrashDump(_exe_game);
    }
    return true;
}
void InitThreadAndDisableDump::InitThreadedLoggerForCPP(std::string &ProjectDirectory, std::string &LogFileName, std::string &GameSaveFolder) {
#pragma warning(push)
#pragma warning(disable : 4996) // Disable warning for getenv
#ifdef _WIN32
    LoggerGlobals::UsernameDirectory = std::getenv("USERNAME");
#else
    LoggerGlobals::UsernameDirectory = std::getenv("USER");
#endif
#pragma warning(pop)

    // this is the folder that contains your src files like main.cpp
    LoggerGlobals::SrcProjectDirectory = ProjectDirectory;
    // Create Log File and folder
    LoggerGlobals::LogFolderPath = "C:\\Users\\" +
                                   LoggerGlobals::UsernameDirectory +
                                   "\\." + GameSaveFolder + "\\logging\\";
    LoggerGlobals::LogFilePath =
        "C:\\Users\\" + LoggerGlobals::UsernameDirectory +
        "\\." + GameSaveFolder + "\\logging\\" + LogFileName + ".log";
    LoggerGlobals::LogFolderBackupPath =
        "C:\\Users\\" + LoggerGlobals::UsernameDirectory +
        "\\." + GameSaveFolder + "\\logging\\LogBackup";
    LoggerGlobals::LogFileBackupPath =
        "C:\\Users\\" + LoggerGlobals::UsernameDirectory +
        "\\." + GameSaveFolder + "\\logging\\LogBackup\\" + LogFileName + "-";

    LoggerThread::GetLoggerThread().StartLoggerThread(
        LoggerGlobals::LogFolderPath, LoggerGlobals::LogFilePath,
        LoggerGlobals::LogFolderBackupPath, LoggerGlobals::LogFileBackupPath);
}