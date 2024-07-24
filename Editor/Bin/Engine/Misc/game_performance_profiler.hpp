#pragma once
// COPYRIGHT MIT : created by https://github.com/quentin452
class GamePerformanceProfiler {
  private:
#if PERFORMANCE_MONITOR
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> startTimes;
    std::unordered_map<std::string, int64_t> profilingData;
    std::unordered_map<std::string, int> callCounts;
    std::unordered_map<std::string, int64_t> maxTimes;
#endif
  public:
#if PERFORMANCE_MONITOR
    Bool stress_test_enabled = true;
#endif
    void start(C std::string &name, C std::string &file, int line, C std::string &customName) {
#if PERFORMANCE_MONITOR
        auto now = std::chrono::steady_clock::now();
        std::string key = customName.empty() ? (name + ":" + file + ":" + std::to_string(line)) : customName;
        startTimes[key] = now;

        if (callCounts.find(key) == callCounts.end()) {
            callCounts[key] = 0;
        }
        callCounts[key]++;
#endif
    }

    void stop(C std::string &name, C std::string &file, int line, C std::string &customName) {
#if PERFORMANCE_MONITOR
        auto now = std::chrono::steady_clock::now();
        std::string key = customName.empty() ? (name + ":" + file + ":" + std::to_string(line)) : customName;

        if (startTimes.find(key) != startTimes.end()) {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - startTimes[key]).count(); // Convertir en microsecondes
            profilingData[key] += duration;

            if (maxTimes.find(key) == maxTimes.end() || duration > maxTimes[key]) {
                maxTimes[key] = duration;
            }

            startTimes.erase(key);

            // double durationSec = duration / 1000000.0;

            // Log the profiling data
            // std::string logMessage = key + ": " + std::to_string(durationSec) + " s";
            // LoggerThread::GetLoggerThread().logMessageAsync(LogLevel::INFO, file.c_str(), line, logMessage);

        } else {
            LoggerThread::GetLoggerThread().logMessageAsync(
                LogLevel::WARNING, file.c_str(), line,
                "Warning: Profiling stopped for a method that was not started.");
        }
#endif
    }

    void print() C {
#if PERFORMANCE_MONITOR
        if (profilingData.empty()) {
            LoggerThread::GetLoggerThread().logMessageAsync(
                LogLevel::WARNING, __FILE__, __LINE__,
                "Warning: profilingData is empty.");
            return;
        }

        double totalProfilingTimeSec = 0;

        std::vector<std::tuple<std::string, double, double, double, double>> sortedData;
        for (C auto &entry : profilingData) {
            std::string key = entry.first;
            double totalTimeSec = entry.second / 1000000.0;
            double avgTimeSec = totalTimeSec / callCounts.at(key);
            double maxTimeSec = maxTimes.at(key) / 1000000.0;
            double combinedAvg = (avgTimeSec + maxTimeSec) / 2.0;
            sortedData.push_back(std::make_tuple(key, totalTimeSec, avgTimeSec, maxTimeSec, combinedAvg));

            totalProfilingTimeSec += totalTimeSec;
        }

        std::sort(sortedData.begin(), sortedData.end(),
                  [](C std::tuple<std::string, double, double, double, double> &a,
                     C std::tuple<std::string, double, double, double, double> &b) {
                      return std::get<1>(b) < std::get<1>(a);
                  });

        std::string totalLogMessage = "Total Profiling Time: " + std::to_string(totalProfilingTimeSec) + " s";
        LoggerThread::GetLoggerThread().logMessageAsync(LogLevel::INFO, __FILE__, __LINE__, totalLogMessage);

        for (C auto &entry : sortedData) {
            std::string key = std::get<0>(entry);
            double totalTimeSec = std::get<1>(entry);
            double avgTimeSec = std::get<2>(entry);
            double maxTimeSec = std::get<3>(entry);
            std::string logMessage = key + ": " + std::to_string(totalTimeSec) + " s (Total Time), " + std::to_string(avgTimeSec) + " s (Average Time For one call), " + std::to_string(maxTimeSec) + " s (Max Time For one call)";
            LoggerThread::GetLoggerThread().logMessageAsync(LogLevel::INFO, __FILE__, __LINE__, logMessage);
            std::string callsMessage = "Calls: " + std::to_string(callCounts.at(key));
            LoggerThread::GetLoggerThread().logMessageAsync(LogLevel::INFO, __FILE__, __LINE__, callsMessage);
        }
#endif
    }

    void addData(C std::string &key, int64_t value) {
#if PERFORMANCE_MONITOR
        profilingData[key] += value;
#endif
    }
};

inline GamePerformanceProfiler gamePerformanceProfiler;

#if PERFORMANCE_MONITOR
#define PROFILE_START(customName) \
    gamePerformanceProfiler.start(__FUNCTION__, __FILE__, __LINE__, customName);

#define PROFILE_STOP(customName) \
    gamePerformanceProfiler.stop(__FUNCTION__, __FILE__, __LINE__, customName);
#else
#define PROFILE_START(customName)
#define PROFILE_STOP(customName)
#endif