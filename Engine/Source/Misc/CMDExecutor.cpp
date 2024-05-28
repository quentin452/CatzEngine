#include "stdafx.h"

namespace EE {
namespace Edit {

#if WINDOWS

HANDLE readPipe = NULL;
HANDLE writePipe = NULL;
HANDLE hSemaphore = NULL;
PROCESS_INFORMATION pi = {};

#endif
CmdExecutor::CmdExecutor() {
#if WINDOWS
    // Setup startup info for cmd.exe
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;

    // Setup security attributes for pipes
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    // Create pipes
    if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) {
        LoggerThread::GetLoggerThread().logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__, "Error: Failed to create pipe.");
    }

    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if (!SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(readPipe);
        CloseHandle(writePipe);
        LoggerThread::GetLoggerThread().logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__, "Error: Failed to set handle information.");
    }

    // Create a semaphore to notify processCommands thread of new commands
    hSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
    if (hSemaphore == NULL) {
        LoggerThread::GetLoggerThread().logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__, "Error: Failed to create semaphore.");
    }

    // Launch cmd.exe
    wchar_t cmdExe[] = L"cmd.exe";
    if (!CreateProcess(NULL,
                       cmdExe,
                       NULL,
                       NULL,
                       TRUE,
                       0,
                       NULL,
                       NULL,
                       &si,
                       &pi)) {
        DWORD errorCode = GetLastError();
        CloseHandle(readPipe);
        CloseHandle(writePipe);
        CloseHandle(hSemaphore);
        LoggerThread::GetLoggerThread().logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__, "Error: Failed to create process. Error code: " + std::to_string(errorCode));
    }

    // Start the processCommands thread
    cmdThread = std::thread(&CmdExecutor::processCommands, this);
    if (!cmdThread.joinable()) {
        CloseHandle(readPipe);
        CloseHandle(writePipe);
        CloseHandle(hSemaphore);
        LoggerThread::GetLoggerThread().logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__, "Error: Failed to create cmdThread.");
    }
#endif
}
CmdExecutor::~CmdExecutor() {
#if WINDOWS
    // Close the command processing thread
    if (cmdThread.joinable()) {
        // Add a command to stop cmd.exe
        executeCommand("exit");
        // Notify processCommands thread that there are new commands to process
        ReleaseSemaphore(hSemaphore, 1, NULL);

        // Wait for the command thread to finish
        cmdThread.join();
    }

    // Close handles
    if (pi.hProcess) {
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
    }

    if (pi.hThread) {
        CloseHandle(pi.hThread);
    }

    if (readPipe) {
        CloseHandle(readPipe);
    }

    if (writePipe) {
        CloseHandle(writePipe);
    }

    if (hSemaphore) {
        CloseHandle(hSemaphore);
    }
#endif
}
bool CmdExecutor::executeCommand(const std::string &command) {
#if WINDOWS
    if (!cmdThread.joinable()) {
        LoggerThread::GetLoggerThread().logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__, "Error: cmdThread is not running.");
        return false;
    }

    LoggerThread::GetLoggerThread().logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__, "Command added to queue: " + command);
    std::lock_guard<std::mutex> lock(commandMutex);
    commandQueue.push(command);

    // Notify processCommands thread that there are new commands to process
    ReleaseSemaphore(hSemaphore, 1, NULL);
#endif
    return true;
}
void CmdExecutor::processCommands() {
#if WINDOWS
    while (true) {
        // Wait for commands to process
        WaitForSingleObject(hSemaphore, INFINITE);

        std::string command;
        // Check if there are commands in the queue
        {
            std::lock_guard<std::mutex> lock(commandMutex);
            if (!commandQueue.empty()) {
                command = commandQueue.front();
                commandQueue.pop();
            }
        }

        if (!command.empty()) {
            // Write the command to the pipe
            DWORD written;
            // Use cmd.exe /C to ensure the command gets executed
            std::string fullCommand = "cmd.exe /C " + command + "\r\n";
            if (!WriteFile(writePipe, fullCommand.c_str(), static_cast<DWORD>(fullCommand.size()), &written, NULL)) {
                LoggerThread::GetLoggerThread().logMessageAsync(
                    LogLevel::ERRORING, __FILE__, __LINE__, "Error: Failed to write command to pipe.");
                continue; // Skip to the next command if writing fails
            } else {
                LoggerThread::GetLoggerThread().logMessageAsync(
                    LogLevel::INFO, __FILE__, __LINE__, "Command written to pipe: " + command);
            }
            // Flush the pipe to ensure command is sent
            FlushFileBuffers(writePipe);

            // Read the output from the read pipe
            std::string output;
            char buffer[4096];
            DWORD bytesRead;
            while (true) {
                BOOL result = ReadFile(readPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
                if (result && bytesRead > 0) {
                    buffer[bytesRead] = '\0'; // Null-terminate the string
                    output += buffer;
                } else {
                    break;
                }
            }

            if (!output.empty()) {
                LoggerThread::GetLoggerThread().logMessageAsync(
                    LogLevel::INFO, __FILE__, __LINE__, "Command output: " + output);
            } else {
                DWORD error = GetLastError();
                LoggerThread::GetLoggerThread().logMessageAsync(
                    LogLevel::ERRORING, __FILE__, __LINE__, "No output received or error reading command output. Error code: " + std::to_string(error));
            }
        }
    }
#endif
}
} // namespace Edit
} // namespace EE