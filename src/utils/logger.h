#pragma once

#include <string>
#include <cstdint>

namespace hotc {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

class Logger {
public:
    static void SetLevel(LogLevel level);
    static void SetOutputFILE(const std::string& path);

    static void Debug(const char* fmt, ...);
    static void Info(const char* fmt, ...);
    static void Warning(const char* fmt, ...);
    static void Error(const char* fmt, ...);
    static void Fatal(const char* fmt, ...);

private:
    static void Log(LogLevel level, const char* fmt, ...);
    static LogLevel current_level_;
};

} // namespace hotc
