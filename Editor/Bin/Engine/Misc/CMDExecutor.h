#pragma once

#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

namespace Edit {
class CmdExecutor {
  public:
    static CmdExecutor &GetInstance() {
        static CmdExecutor instance;
        return instance;
    }

    Bool executeCommand(C std::string &command, bool outputEnabled = true);
    Bool isCommandFinished();
    Bool isCmdIdle();
    void stopCmdProcess();

    // Delete copy and move constructors, assignment operators
    CmdExecutor(CmdExecutor C &) = delete;
    void operator=(CmdExecutor C &) = delete;

    std::mutex commandMutex;
    std::mutex isCommandExecutingMutex;

  private:
    CmdExecutor();
    ~CmdExecutor();

    void processCommands();

    std::string command;
    std::queue<std::pair<std::string, bool>> commandQueue;
    std::thread cmdThread;
    std::condition_variable commandCv;

    Bool isCommandExecuting;
};

} // namespace Edit