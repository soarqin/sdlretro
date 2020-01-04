#pragma once

#include <cstdarg>
#include <ostream>

namespace drivers {

enum log_level: int {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
};

void log_init(const std::string &filename);
void log_vprintf(int level, const char *fmt, va_list arg);

std::ostream &logger(int log_level);

}
