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

CmdExecutor::CmdExecutor() {
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
    si.wShowWindow = SW_SHOW;
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
    // Close the command processing thread
    if (cmdThread.joinable()) {
        // Add a command to stop cmd.exe
        executeCommand("exit");
        // Wait for the command thread to finish
        cmdThread.join();
    }

    // Close handles
    if (pi.hProcess) {
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
    }
    CloseHandle(pi.hThread);
    CloseHandle(childStdOutRd);
    CloseHandle(childStdOutWr);
    CloseHandle(childStdInRd);
    CloseHandle(childStdInWr);
#endif
}

bool CmdExecutor::executeCommand(const std::string &command) {
#if WINDOWS
    LoggerThread::GetLoggerThread().logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__, "Command added to queue: " + command);
    {
        std::lock_guard<std::mutex> lock(commandMutex);
        commandQueue.push(command);
    }
    commandCv.notify_one();
#endif
    return true;
}

void CmdExecutor::processCommands() {
#if WINDOWS
    // Create a thread to read continuously from the child process output
    std::thread outputThread(&CmdExecutor::readOutput, this);

    while (true) {
        std::string command;
        {
            std::unique_lock<std::mutex> lock(commandMutex);
            if (commandQueue.empty()) {
                // Wait for new commands to be added to the queue
                commandCv.wait(lock, [this] { return !commandQueue.empty(); });
            }
            command = std::move(commandQueue.front());
            commandQueue.pop();
        }

        if (command == "exit") {
            break;
        }

        // Write the command to the pipe
        DWORD written;
        std::string fullCommand = command + "\r\n";
        if (!WriteFile(childStdInWr, fullCommand.c_str(), static_cast<DWORD>(fullCommand.size()), &written, NULL)) {
            LoggerThread::GetLoggerThread().logMessageAsync(
                LogLevel::ERRORING, __FILE__, __LINE__, "Error: Failed to write command to pipe.");
            // Log the command that failed to be written
            LoggerThread::GetLoggerThread().logMessageAsync(
                LogLevel::ERRORING, __FILE__, __LINE__, "Failed command: " + command);
            continue; // Skip to the next command if writing fails
        } else {
            // Log that the command was successfully written
            LoggerThread::GetLoggerThread().logMessageAsync(
                LogLevel::INFO, __FILE__, __LINE__, "Command written to pipe: " + command);
        }
        // Flush the pipe to ensure command is sent
        FlushFileBuffers(childStdInWr);
    }

    // Join the output thread before exiting
    DoForceReload = false;
    outputThread.join();
#endif
}

void CmdExecutor::readOutput() {
#if WINDOWS
    std::string output;
    char buffer[4096];
    DWORD bytesRead;
    BOOL result;

    // Read the command output until there is no more data
    do {
        result = ReadFile(childStdOutRd, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        if (result && bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Null-terminate the string
            // Append the buffer to output
            output.append(buffer, bytesRead);

            // Check if the output contains a complete command response
            size_t pos;
            while ((pos = output.find("\r\n")) != std::string::npos) {
                std::string commandOutput = output.substr(0, pos);
                // Log the command output
                if (!commandOutput.empty()) {
                    // Remove trailing '\r' and '\n' characters
                    commandOutput.erase(std::remove(commandOutput.begin(), commandOutput.end(), '\r'), commandOutput.end());
                    commandOutput.erase(std::remove(commandOutput.begin(), commandOutput.end(), '\n'), commandOutput.end());

                    LoggerThread::GetLoggerThread().logMessageAsync(
                        LogLevel::INFO, __FILE__, __LINE__, "Command output: " + commandOutput);
                }
                // Erase the processed output from the buffer
                output.erase(0, pos + 2);
            }
        }
    } while (result && bytesRead > 0);
#endif
}

} // namespace Edit
} // namespace EE
