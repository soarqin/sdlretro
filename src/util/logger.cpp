/*
 * Copyright (c) 2022 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "logger.h"

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdio>

namespace util {

static bool sPreamable = true;
static bool sForceFlush = true;
static bool sUseColor = false;
static auto sBootTime = std::chrono::steady_clock::now();
static thread_local const char *sThreadName = nullptr;
/* 0 not in line
 * 1 line head
 * 2 line body
 */
static thread_local std::uint8_t sInLine = 0;
static thread_local LogLevel sLineLogLevel = LogLevel::NONE;
static thread_local bool sLineHasStyle = false;

static FILE* sConsoleOutput = nullptr;
static std::vector<std::pair<FILE*, LogLevel>> sFileOutput;

static LogLevel sConsoleLogLevel = LogLevel::INFO;
static LogLevel sMaxLogLevel = LogLevel::NONE;

struct LogQueueItem {
    LogLevel level;
    fmt::basic_memory_buffer<char> buf;
};

static std::vector<LogQueueItem> sLogQueue[2];
static std::mutex sLogMutex;
static std::thread sLogThread;
static std::condition_variable sLogCV;
static bool sLogThreadRunning = false;

static inline void logMessageInternal(LogQueueItem &item) {
    if (item.level <= sConsoleLogLevel) {
        std::fputs(item.buf.data(), sConsoleOutput);
        if (sForceFlush) {
            std::fflush(sConsoleOutput);
        }
    }
    for (auto p: sFileOutput) {
        if (item.level > p.second) { continue; }
        std::fputs(item.buf.data(), p.first);
        if (sForceFlush) {
            std::fflush(p.first);
        }
    }
}

LogLevel logLevelFromString(const std::string &name) {
    if (name == "none") return LogLevel::NONE;
    if (name == "fatal") return LogLevel::FATAL;
    if (name == "error") return LogLevel::ERROR;
    if (name == "warn") return LogLevel::WARN;
    if (name == "info") return LogLevel::INFO;
    if (name == "debug") return LogLevel::DEBUG;
    if (name == "trace") return LogLevel::TRACE;
    return LogLevel::VERBOSE_L1;
}

void logInit(FILE *fd, LogLevel level) {
    sConsoleOutput = fd;
    sConsoleLogLevel = level;
    sMaxLogLevel = level;
    sUseColor = fd != nullptr;
    sLogThreadRunning = true;
    sLogThread = std::thread([]() {
        while (true) {
            std::unique_lock lk(sLogMutex);
            using namespace std::chrono_literals;
            sLogCV.wait_for(lk, 20ms);
            if (!sLogThreadRunning) { break; }
            if (sLogQueue[0].empty()) { continue; }
            std::swap(sLogQueue[0], sLogQueue[1]);
            lk.unlock();
            for (auto &li: sLogQueue[1]) {
                logMessageInternal(li);
            }
            sLogQueue[1].clear();
        }
    });
}

void logUninit() {
    sLogThreadRunning = false;
    sLogCV.notify_all();
    sLogThread.join();
}

void logAdd(FILE *fd, LogLevel level) {
    if (fd == stderr || fd == stdout) {
        sConsoleOutput = fd;
        sConsoleLogLevel = level;
        return;
    }
    sFileOutput.emplace_back(std::make_pair(fd, level));
    sMaxLogLevel = std::max(sMaxLogLevel, level);
}

static const char *LogLevelToString(LogLevel ll) {
    static const char *names[std::size_t(LogLevel::Max)] = {
        "",
        "FATAL",
        "ERROR",
        "WARN",
        "INFO",
        "DEBUG",
        "TRACE",
    };
    return names[std::size_t(ll)];
}

static const fmt::text_style &LogLevelToStyle(LogLevel ll) {
    static fmt::text_style styles[std::size_t(LogLevel::Max)] = {
        fmt::text_style(),
        fg(fmt::color::white) | bg(fmt::color::red) | fmt::emphasis::bold,
        fg(fmt::color::red) | fmt::emphasis::bold,
        fg(fmt::color::yellow) | fmt::emphasis::bold,
        fg(fmt::color::green),
        fg(fmt::color::cyan),
    };
    return styles[std::size_t(ll)];
}

void logMessage(LogLevel level, const char *format, fmt::format_args args) {
    if (level > sMaxLogLevel) { return; }
    bool hasStyle = false;
    fmt::basic_memory_buffer<char> buf;
    auto bufIt = std::back_inserter(buf);
    auto inLine = sInLine;
    if (inLine < 2) {
        if (sUseColor) {
            const auto &ts = LogLevelToStyle(level);
            if (ts.has_emphasis()) {
                auto emphasis = fmt::detail::make_emphasis<char>(ts.get_emphasis());
                buf.append(emphasis.begin(), emphasis.end());
                hasStyle = true;
            }
            if (ts.has_foreground()) {
                auto foreground = fmt::detail::make_foreground_color<char>(ts.get_foreground());
                buf.append(foreground.begin(), foreground.end());
                hasStyle = true;
            }
            if (ts.has_background()) {
                auto background = fmt::detail::make_background_color<char>(ts.get_background());
                buf.append(background.begin(), background.end());
                hasStyle = true;
            }
        }
        if (sPreamable && level != LogLevel::NONE) {
            auto mill =
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - sBootTime);
            if (sThreadName) {
                fmt::format_to(bufIt,
                               "{:%y%m%d %H:%M:%S} ({:4}.{:03}) [{}] {:5}|",
                               std::chrono::system_clock::now(),
                               mill.count() / 1000,
                               mill.count() % 1000,
                               sThreadName,
                               LogLevelToString(level));
            } else {
                fmt::format_to(bufIt,
                               "{:%y%m%d %H:%M:%S} ({:4}.{:03}) {:5}|",
                               std::chrono::system_clock::now(),
                               mill.count() / 1000,
                               mill.count() % 1000,
                               LogLevelToString(level));
            }
        }
        if (inLine == 1) {
            sLineHasStyle = hasStyle;
        }
    }
    fmt::vformat_to(bufIt, std::string_view(format), args);
    if (inLine == 0) {
        if (hasStyle) {
            fmt::detail::reset_color(buf);
        }
        static const char *tail = "\n";
        buf.append(tail, tail + 2);
    } else {
        buf.push_back(0);
    }
    std::lock_guard lk(sLogMutex);
    auto &lq = sLogQueue[0];
    lq.resize(lq.size() + 1);
    auto &li = lq.back();
    li.level = level;
    li.buf = std::move(buf);
    sLogCV.notify_one();
}

void logMessageN(const char *format, fmt::format_args args) {
    if (sInLine == 0) { return; }
    logMessage(sLineLogLevel, format, args);
}


void logSetPreamable(bool preamable) {
    sPreamable = preamable;
}

void logSetUseColor(bool useColor) {
    sUseColor = useColor;
}

void logSetForceFlush(bool forceFlush) {
    sForceFlush = forceFlush;
}

void logSetThreadName(const char *name) {
    sThreadName = name;
}

void logBeginLine(LogLevel level) {
    if (sInLine != 0) { return; }
    if (level > sMaxLogLevel) {
        return;
    }
    sInLine = 1;
    logMessage(level, "", {});
    sInLine = 2;
    sLineLogLevel = level;
    sLineHasStyle = false;
}

void logEndLine() {
    if (sInLine == 0) { return; }
    fmt::basic_memory_buffer<char> buf;
    if (sLineHasStyle) {
        fmt::detail::reset_color(buf);
    }
    static const char *tail = "\n";
    buf.append(tail, tail + 2);
    std::lock_guard lk(sLogMutex);
    auto &lq = sLogQueue[0];
    lq.resize(lq.size() + 1);
    auto &li = lq.back();
    li.level = LogLevel::NONE;
    li.buf = std::move(buf);
    sLogCV.notify_one();
    sInLine = 0;
    sLineLogLevel = LogLevel::NONE;
    sLineHasStyle = false;
}

}
