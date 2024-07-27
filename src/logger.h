#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>

// Define log levels
enum class LogLevel {
    ERROR = 0,
    INFO = 1,
    DEBUG = 2
};

// Use the compile-time LOG_LEVEL definition
#ifndef LOG_LEVEL
#define LOG_LEVEL 2  // Default to DEBUG if not defined
#endif

#define COMPILE_TIME_LOG_LEVEL static_cast<LogLevel>(LOG_LEVEL)

class Logger {
public:
    class LogStream {
    public:
        LogStream(LogLevel level, const char* file, int line, const char* function)
            : level_(level), file_(file), line_(line), function_(function) {}

        ~LogStream() {
            Logger::log(level_, oss_.str(), file_, line_, function_);
        }

        template<typename T>
        LogStream& operator<<(const T& value) {
            oss_ << value;
            return *this;
        }

    private:
        LogLevel level_;
        const char* file_;
        int line_;
        const char* function_;
        std::ostringstream oss_;
    };

    static void log(LogLevel level, const std::string& message, const char* file, int line, const char* function) {
        if (level <= COMPILE_TIME_LOG_LEVEL) {
            std::ostringstream oss;
            oss << "[" << get_current_time() << "]"
                << "[" << get_log_level_string(level) << "]"
                << "[" << file << ":" << line << "]"
                << "[" << function << "()]"
                << " - "
                << message;
            std::cout << oss.str() << std::endl;
        }
    }

private:
    static std::string get_current_time() {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    static const char* get_log_level_string(LogLevel level) {
        switch (level) {
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::INFO:  return "INFO";
            case LogLevel::DEBUG: return "DEBUG";
            default:              return "UNKNOWN";
        }
    }
};

// Convenience macros
#define LOG_ERROR Logger::LogStream(LogLevel::ERROR, __FILE__, __LINE__, __func__)
#define LOG_INFO Logger::LogStream(LogLevel::INFO, __FILE__, __LINE__, __func__)
#define LOG_DEBUG Logger::LogStream(LogLevel::DEBUG, __FILE__, __LINE__, __func__)