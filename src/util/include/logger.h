/*
 * Copyright (c) 2022 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <fmt/core.h>

#include <cstdio>
#include <cstdint>

namespace util {

enum class LogLevel: std::size_t {
    NONE,
    FATAL,
    Fatal = FATAL,
    ERROR,
    Error = ERROR,
    WARN,
    Warn = WARN,
    INFO,
    Info = INFO,
    DEBUG,
    Debug = DEBUG,
    TRACE,
    Trace = TRACE,
    VERBOSE_L1,
    VERBOSE_L2,
    VERBOSE_L3,
    VERBOSE_L4,
    VERBOSE_L5,
    VERBOSE_L6,
    VERBOSE_L7,
    VERBOSE_L8,
    VERBOSE_L9,
    Max,
};

LogLevel logLevelFromString(const std::string &name);

void logInit(FILE *fd = stdout, LogLevel level = LogLevel::INFO);
void logUninit();
void logAdd(FILE *fd, LogLevel level = LogLevel::INFO);

void logMessage(LogLevel level, const char *format, fmt::format_args args);
void logMessageN(const char *format, fmt::format_args args);
void logSetPreamable(bool preamable = true);
void logSetUseColor(bool useColor = true);
void logSetForceFlush(bool forceFlush = true);
void logSetThreadName(const char *name);

void logBeginLine(LogLevel level);
void logEndLine();

template<typename... Args>
inline void vlogMessage(LogLevel level, const char *format, const Args&... args) {
    return logMessage(level, format, fmt::make_format_args(args...));
}

template<typename... Args>
inline void vlogMessageN(const char *format, const Args&... args) {
    return logMessageN(format, fmt::make_format_args(args...));
}

struct LogScopedLine final {
    LogScopedLine(LogLevel level) {
        logBeginLine(level);
    }
    ~LogScopedLine() {
        logEndLine();
    }
};

}

#define LOG(level, fmt, ...) ::util::vlogMessage(::util::LogLevel::level, fmt, ##__VA_ARGS__)
#define LOGN(fmt, ...) ::util::vlogMessageN(fmt, ##__VA_ARGS__)
#define LOG_IF(cond, level, fmt, ...) if (cond) { ::util::vlogMessage(::util::LogLevel::level, fmt, ##__VA_ARGS__); }
#define LOG_SCOPEDLINE(level) ::util::LogScopedLine scopedLine__(::util::LogLevel::level)
