#ifndef LOGGER_THREAD_HPP
#define LOGGER_THREAD_HPP

#include "LoggerFileSystem.hpp"
#include "LoggerGlobals.hpp"

#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

enum class LogLevel { INFO,
                      WARNING,
                      ERRORING,
                      LOGICERROR };

class LoggerThread {
#if defined(_WIN32) || defined(__linux__) || !defined(__ANDROID__) || !defined(EMSCRIPTEN) || !defined(__NINTENDO__) || !defined(TARGET_OS_IPHONE)

  public:
    LoggerThread() : Done_Logger_Thread(false) {
        workerThread = std::thread(&LoggerThread::logWorker, this);
    }

    ~LoggerThread() {
        cleanup();
    }

    LoggerThread(LoggerThread &&other) noexcept
        : workerThread(std::move(other.workerThread)),
          tasks(std::move(other.tasks)),
          mtx(),
          Unlock_Logger_Thread(),
          Done_Logger_Thread(other.Done_Logger_Thread.load()), // use load for atomic bool
          logFile(std::move(other.logFile)),
          logFilePath_(std::move(other.logFilePath_)),
          LogFolderPathForTheThread(std::move(other.LogFolderPathForTheThread)),
          LogFilePathForTheThread(std::move(other.LogFilePathForTheThread)),
          LogFolderBackupPathForTheThread(std::move(other.LogFolderBackupPathForTheThread)),
          LogFileBackupPathForTheThread(std::move(other.LogFileBackupPathForTheThread)),
          TimeStamp(std::move(other.TimeStamp)) {
        other.Done_Logger_Thread = true;
        if (!workerThread.joinable()) {
            workerThread = std::thread(&LoggerThread::logWorker, this);
        }
    }

    LoggerThread &operator=(LoggerThread &&other) noexcept {
        if (this != &other) {
            cleanup();
            workerThread = std::move(other.workerThread);
            tasks = std::move(other.tasks);
            Done_Logger_Thread = other.Done_Logger_Thread.load(); // use load for atomic bool
            logFile = std::move(other.logFile);
            logFilePath_ = std::move(other.logFilePath_);
            LogFolderPathForTheThread = std::move(other.LogFolderPathForTheThread);
            LogFilePathForTheThread = std::move(other.LogFilePathForTheThread);
            LogFolderBackupPathForTheThread = std::move(other.LogFolderBackupPathForTheThread);
            LogFileBackupPathForTheThread = std::move(other.LogFileBackupPathForTheThread);
            TimeStamp = std::move(other.TimeStamp);
            other.Done_Logger_Thread = true;
            if (!workerThread.joinable()) {
                workerThread = std::thread(&LoggerThread::logWorker, this);
            }
        }
        return *this;
    }

    static LoggerThread &GetLoggerThread() {
        if (LoggerInstanceT == nullptr) {
            LoggerInstanceT = std::unique_ptr<LoggerThread>(new LoggerThread());
        }
        return *LoggerInstanceT;
    }

    void logMessageAsync(LogLevel level, const std::string &sourceFile, int line, const std::string &message) {
        logMessageAsync(level, sourceFile, line, {message});
    }

    void logMessageAsync(LogLevel level, const std::string &sourceFile, int line, const std::initializer_list<std::string> &messageParts) {
        std::ostringstream messageStream;
        for (const auto &part : messageParts) {
            messageStream << part;
        }
        std::string message = messageStream.str();

        {
            std::unique_lock<std::mutex> lock(mtx);
            tasks.emplace([=] { logMessage(level, sourceFile, line, message); });
        }
        Unlock_Logger_Thread.notify_one();
    }

    void ExitLoggerThread() {
        cleanup();
        // Perform cleanup tasks
        TimeStamp = getTimestamp();
        std::string src = LogFilePathForTheThread;
        std::string dst = LogFileBackupPathForTheThread + TimeStamp + ".log";
        copyFile(src, dst);
    }

    void StartLoggerThread(const std::string &LogFolderPath, const std::string &LogFilePath, const std::string &LogFolderBackupPath, const std::string &LogFileBackupPath) {
        this->LogFolderPathForTheThread = LogFolderPath;
        this->LogFilePathForTheThread = LogFilePath;
        this->logFilePath_ = LogFilePath;
        this->LogFolderBackupPathForTheThread = LogFolderBackupPath;
        this->LogFileBackupPathForTheThread = LogFileBackupPath;
        LoggerFileSystem::createDirectories(LogFolderBackupPathForTheThread);
        LoggerFileSystem::createDirectories(LogFolderPathForTheThread);
        LoggerFileSystem::createFile(LogFilePathForTheThread);
        logFile.open(logFilePath_, std::ios::out | std::ios::trunc); // Open file in truncate mode
        if (!logFile.is_open()) {
            std::cerr << "Error: Unable to open log file.\n";
        }
    }

  private:
    static std::unique_ptr<LoggerThread> LoggerInstanceT;
    std::thread workerThread;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable Unlock_Logger_Thread;
    std::atomic<bool> Done_Logger_Thread;
    std::ofstream logFile;
    std::string logFilePath_;
    std::string LogFolderPathForTheThread;
    std::string LogFilePathForTheThread;
    std::string LogFolderBackupPathForTheThread;
    std::string LogFileBackupPathForTheThread;
    std::string TimeStamp;

    void logWorker() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mtx);
                Unlock_Logger_Thread.wait(lock, [this] { return !tasks.empty() || Done_Logger_Thread; });
                if (Done_Logger_Thread && tasks.empty()) {
                    break;
                }
                task = std::move(tasks.front());
                tasks.pop();
            }
            task();
        }
    }

    template <typename... Args>
    void logMessage(LogLevel level, const std::string &sourceFile, int line, const Args &...args) {
        std::ostringstream oss;
        switch (level) {
        case LogLevel::INFO:
            oss << "[INFO] ";
            break;
        case LogLevel::WARNING:
            oss << "[WARNING] ";
            break;
        case LogLevel::ERRORING:
            oss << "[ERROR] ";
            break;
        case LogLevel::LOGICERROR:
            oss << "[LOGIC ERROR] ";
            break;
        }
        oss << getTimestamp() << " [" << extractRelativePath(sourceFile) << ":" << line << "] ";
        append(oss, args...);
        std::string message = oss.str();
        std::cout << message << std::endl;
        if (logFile.is_open()) {
            std::lock_guard<std::mutex> lock(mtx); // Lock before writing
            logFile << message << std::endl;       // Write to file
        } else {
            std::cerr << "Error: Unable to write to log file.\n";
        }
    }

    std::string extractRelativePath(const std::string &filePath) {
        size_t found = filePath.find_last_of("/\\");
        if (found != std::string::npos) {
            size_t srcIndex = filePath.rfind(LoggerGlobals::SrcProjectDirectory, found);
            if (srcIndex != std::string::npos) {
                return filePath.substr(srcIndex);
            }
            return filePath.substr(found);
        }
        return filePath;
    }

    template <typename T>
    void append(std::ostringstream &oss, const T &arg) {
        oss << arg;
    }

    template <typename T, typename... Args>
    void append(std::ostringstream &oss, const T &first, const Args &...args) {
        oss << first;
        append(oss, args...);
    }

    void copyFile(const std::string &source, const std::string &dest) {
        std::ifstream src(source, std::ios::binary);
        if (!src.is_open()) {
            std::cerr << "Error: Unable to open source file " << source << ".\n";
            return;
        }

        std::ofstream dst(dest, std::ios::binary);
        if (!dst.is_open()) {
            std::cerr << "Error: Unable to create or open destination file " << dest << ".\n";
            src.close();
            return;
        }
        dst << src.rdbuf(); // Efficiently copy file
        src.close();
        dst.close();
    }

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        struct tm timeinfo;

#ifdef _WIN32
        localtime_s(&timeinfo, &in_time_t);
#else
        localtime_r(&timeinfo, &in_time_t);
#endif

        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d-%H-%M-%S", &timeinfo);

        return std::string(buffer);
    }

    void cleanup() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            Done_Logger_Thread = true;
            Unlock_Logger_Thread.notify_one(); // Notify worker thread to stop
        }
        if (workerThread.joinable()) {
            workerThread.join(); // Wait for worker thread to finish
        }
        if (logFile.is_open()) {
            logFile.close();
        }
    }

#else
  public:
    LoggerThread() {}

    ~LoggerThread() {}

    void logMessageAsync(LogLevel level, const std::string &sourceFile, int line, const std::string &message) {}

    void logMessageAsync(LogLevel level, const std::string &sourceFile, int line, const std::initializer_list<std::string> &messageParts) {}

    void ExitLoggerThread() {}

    void StartLoggerThread(const std::string &LogFolderPath, const std::string &LogFilePath, const std::string &LogFolderBackupPath, const std::string &LogFileBackupPath) {}

  private:
    void logWorker() {}

    template <typename... Args>
    void logMessage(LogLevel level, const std::string &sourceFile, int line, const Args &...args) {}

    template <typename T>
    void append(std::ostringstream &oss, const T &arg) {}

    template <typename T, typename... Args>
    void append(std::ostringstream &oss, const T &first, const Args &...args) {}

    void copyFile(const std::string &source, const std::string &dest) {}
#endif
};
#endif // LOGGER_THREAD_HPP
