#include "logger.h"
#include <cstdarg>
#include <cstdio>
#include <ctime>

namespace hotc {

LogLevel Logger::current_level_ = LogLevel::Info;

void Logger::SetLevel(LogLevel level) {
    current_level_ = level;
}

void Logger::SetOutputFILE(const std::string& path) {
    // Would open file for output
}

void Logger::Debug(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Log(LogLevel::Debug, fmt, args);
    va_end(args);
}

void Logger::Info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Log(LogLevel::Info, fmt, args);
    va_end(args);
}

void Logger::Warning(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Log(LogLevel::Warning, fmt, args);
    va_end(args);
}

void Logger::Error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Log(LogLevel::Error, fmt, args);
    va_end(args);
}

void Logger::Fatal(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Log(LogLevel::Fatal, fmt, args);
    va_end(args);
}

void Logger::Log(LogLevel level, const char* fmt, ...) {
    if (level < current_level_) return;

    const char* level_str;
    switch (level) {
        case LogLevel::Debug: level_str = "DEBUG"; break;
        case LogLevel::Info: level_str = "INFO"; break;
        case LogLevel::Warning: level_str = "WARN"; break;
        case LogLevel::Error: level_str = "ERROR"; break;
        case LogLevel::Fatal: level_str = "FATAL"; break;
        default: level_str = "UNKNOWN"; break;
    }

    time_t now = time(nullptr);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(stderr, "[%s] [%s] ", time_buf, level_str);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");
}

} // namespace hotc
