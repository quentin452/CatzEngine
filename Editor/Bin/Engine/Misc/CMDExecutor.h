#pragma once

#include "stdafx.h"
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

    Bool executeCommand(C std::string &command);
    Bool isCommandFinished();
    Bool isCmdIdle();
    void stopCmdProcess();

    // Delete copy and move constructors, assignment operators
    CmdExecutor(CmdExecutor C &) = delete;
    void operator=(CmdExecutor C &) = delete;

    std::mutex commandMutex;

  private:
    CmdExecutor();
    ~CmdExecutor();

    void processCommands();

    std::string command;
    std::queue<std::string> commandQueue;
    std::thread cmdThread;
    std::condition_variable commandCv;
};

} // namespace Edit