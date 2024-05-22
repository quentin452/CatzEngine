#pragma once
#include "LoggerThread.hpp"
#include <thread>

#include <string>
class GlobalsLoggerInstance {
public:
  inline static LoggerThread LoggerInstance;
};
