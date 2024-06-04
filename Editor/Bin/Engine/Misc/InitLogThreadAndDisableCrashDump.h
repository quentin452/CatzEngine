#pragma once
struct InitThreadAndDisableDump {
    static std::wstring ConvertToWideString(const std::string &narrow_str);
    static long DisableMemoryCrashDump(std::string _exe_name);
    static bool elevatePrivileges(const char *exePath);
    static bool GetExeNameAndCheckAdminPrivileges(char *exePath, char *exeName, bool *hasAdminPrivileges);
    static bool Init_elevatePrivile_DisableMemoryCrashDump_For_Games(std::string &_exe_game);
    static void InitThreadedLoggerForCPP(std::string &ProjectDirectory, std::string &LogFileName, std::string &GameSaveFolder);
};