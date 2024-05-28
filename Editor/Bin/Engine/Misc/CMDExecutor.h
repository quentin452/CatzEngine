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

    bool executeCommand(const std::string &command);

    // Delete copy and move constructors, assignment operators
    CmdExecutor(CmdExecutor const &) = delete;
    void operator=(CmdExecutor const &) = delete;

  private:
    CmdExecutor();
    ~CmdExecutor();

    void processCommands();

  private:
    std::queue<std::string> commandQueue;
    std::mutex commandMutex;
    std::thread cmdThread;
    std::condition_variable commandCv;
};

} // namespace Edit