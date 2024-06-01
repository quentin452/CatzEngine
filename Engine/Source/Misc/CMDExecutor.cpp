#include "stdafx.h"

namespace EE {
namespace Edit {

#if WINDOWS

HANDLE childStdOutRd = NULL;
HANDLE childStdOutWr = NULL;
HANDLE childStdInRd = NULL;
HANDLE childStdInWr = NULL;
PROCESS_INFORMATION pi = {};

#endif

CmdExecutor::CmdExecutor() : isCommandExecuting(false) {
#if WINDOWS
    // Setup security attributes for pipes
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    // Create pipes for STDOUT
    if (!CreatePipe(&childStdOutRd, &childStdOutWr, &sa, 0)) {
        LoggerThread::GetLoggerThread().logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__, "Error: Failed to create pipe for STDOUT.");
    }
    if (!SetHandleInformation(childStdOutRd, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(childStdOutRd);
        CloseHandle(childStdOutWr);
        LoggerThread::GetLoggerThread().logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__, "Error: Failed to set handle information for STDOUT.");
    }

    // Create pipes for STDIN
    if (!CreatePipe(&childStdInRd, &childStdInWr, &sa, 0)) {
        LoggerThread::GetLoggerThread().logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__, "Error: Failed to create pipe for STDIN.");
    }
    if (!SetHandleInformation(childStdInWr, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(childStdInRd);
        CloseHandle(childStdInWr);
        LoggerThread::GetLoggerThread().logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__, "Error: Failed to set handle information for STDIN.");
    }

    // Setup startup info for cmd.exe
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = childStdOutWr;
    si.hStdInput = childStdInRd;
    si.hStdError = childStdOutWr;

    // Get the current directory
    wchar_t currentDirectory[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentDirectory);

    // Launch cmd.exe
    wchar_t cmdExe[] = L"cmd.exe";
    if (!CreateProcess(NULL,
                       cmdExe,
                       NULL,
                       NULL,
                       TRUE,
                       0,
                       NULL,
                       currentDirectory,
                       &si,
                       &pi)) {
        DWORD errorCode = GetLastError();
        CloseHandle(childStdOutRd);
        CloseHandle(childStdOutWr);
        CloseHandle(childStdInRd);
        CloseHandle(childStdInWr);
        LoggerThread::GetLoggerThread().logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__, "Error: Failed to create process. Error code: " + std::to_string(errorCode));
    }
    // Start the processCommands thread
    cmdThread = std::thread(&CmdExecutor::processCommands, this);
#endif
}

CmdExecutor::~CmdExecutor() {
#if WINDOWS
    stopCmdProcess();
#endif
}

Bool CmdExecutor::executeCommand(C std::string& command, bool outputEnabled) {
#if WINDOWS
    if (isCommandExecuting) {
        LoggerThread::GetLoggerThread().logMessageAsync(
            LogLevel::INFO, __FILE__, __LINE__, "Command execution is already in progress.");
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(commandMutex);
        commandQueue.push(std::make_pair(command, outputEnabled));
    }
    commandCv.notify_one();
    return true;
#endif
}

Bool CmdExecutor::isCommandFinished() {
#if WINDOWS
    DWORD exitCode;
    if (!GetExitCodeProcess(pi.hProcess, &exitCode)) {
        LoggerThread::GetLoggerThread().logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__, "Error: Failed to get exit code of process.");
        return false;
    }
    return exitCode != STILL_ACTIVE;
#else
    return true;
#endif
}

Bool CmdExecutor::isCmdIdle() {
#if WINDOWS
    // Check if the cmd process is not active
    return !isCommandFinished();
#else
    return true;
#endif
}

void CmdExecutor::processCommands() {
#if WINDOWS
    while (true) {
        try {
            bool outputEnabled = false;
            {
                std::unique_lock<std::mutex> lock(commandMutex);
                if (commandQueue.empty()) {
                    LoggerThread::GetLoggerThread().logMessageAsync(
                        LogLevel::INFO, __FILE__, __LINE__, "Command queue is empty. Waiting for new commands.");
                    commandCv.wait(lock, [this] { return !commandQueue.empty(); });
                }
                if (isCommandExecuting) {
                    continue;
                }
                isCommandExecuting = true;
                std::pair<std::string, bool> commandPair = std::move(commandQueue.front());
                commandQueue.pop();
                command = commandPair.first;
                bool outputEnabled = commandPair.second;
                LoggerThread::GetLoggerThread().logMessageAsync(
                    LogLevel::INFO, __FILE__, __LINE__, "Command dequeued: " + command);
            }

            if (command == "exit") {
                LoggerThread::GetLoggerThread().logMessageAsync(
                    LogLevel::INFO, __FILE__, __LINE__, "Exit command received. Breaking the loop.");
                break;
            } // Write the command to the pipe
            DWORD written;
            std::string fullCommand = command + "\r\n";
            if (!WriteFile(childStdInWr, fullCommand.c_str(), static_cast<DWORD>(fullCommand.size()), &written, NULL)) {
                LoggerThread::GetLoggerThread().logMessageAsync(
                    LogLevel::ERRORING, __FILE__, __LINE__, "Error: Failed to write command to pipe.");
                // Log the command that failed to be written
                LoggerThread::GetLoggerThread().logMessageAsync(
                    LogLevel::ERRORING, __FILE__, __LINE__, "Failed command: " + command);
                isCommandExecuting = false;
                continue; // Skip to the next command if writing fails
            } else {
                // Log that the command was successfully written
                LoggerThread::GetLoggerThread().logMessageAsync(
                    LogLevel::INFO, __FILE__, __LINE__, "Command written to pipe: " + command);
            }

            // Flush the pipe to ensure command is sent
            FlushFileBuffers(childStdInWr);
            LoggerThread::GetLoggerThread().logMessageAsync(
                LogLevel::INFO, __FILE__, __LINE__, "Pipe flushed after writing command.");

            DWORD exitCode;
            if (!GetExitCodeProcess(pi.hProcess, &exitCode)) {
                LoggerThread::GetLoggerThread().logMessageAsync(
                    LogLevel::ERRORING, __FILE__, __LINE__, "Error: Failed to get exit code of process.");
                isCommandExecuting = false;
                continue;
            }
            if (exitCode != STILL_ACTIVE) {
                LoggerThread::GetLoggerThread().logMessageAsync(
                    LogLevel::ERRORING, __FILE__, __LINE__, "Error: Process is no longer running.");
                break;
            }
            DWORD bytesRead = 0;
            BOOL success = FALSE;
            C int BUFFER_SIZE = 4096;
            char buffer[BUFFER_SIZE];
            while (PeekNamedPipe(childStdOutRd, NULL, 0, NULL, &bytesRead, NULL) && bytesRead > 0) {
                success = ReadFile(childStdOutRd, buffer, BUFFER_SIZE, &bytesRead, NULL);
                if (!success || bytesRead == 0) {
                    break;
                }

                if (outputEnabled) {
                    std::string output(buffer, bytesRead);
                    LoggerThread::GetLoggerThread().logMessageAsync(
                        LogLevel::INFO, __FILE__, __LINE__, "Command output: " + output);
                }
            }

            isCommandExecuting = false;
        } catch (...) {
            isCommandExecuting = false;
            throw;
        }
    }
#endif
}
void CmdExecutor::stopCmdProcess() {
#if WINDOWS
    if (pi.hProcess) {
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        pi.hProcess = NULL;
    }
    if (pi.hThread) {
        CloseHandle(pi.hThread);
    }
#endif
}
} // namespace Edit
} // namespace EE
