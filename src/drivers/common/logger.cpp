#include "logger.h"

#include <cstdio>
#include <ctime>

#include <iostream>
#include <fstream>

namespace drivers {

static std::ostream *log_stream = nullptr;

void log_init(const std::string &filename) {
    if (filename.empty())
        log_stream = &std::cout;
    else {
        log_stream = new std::ofstream(filename, std::ios_base::ate | std::ios_base::out);
        if (!log_stream->good()) {
            delete log_stream;
            log_stream = &std::cout;
        } else {
            log_stream->rdbuf()->pubsetbuf(nullptr, 0);
        }
    }
}

void log_vprintf(int level, const char *fmt, va_list arg) {
    if (!log_stream) return;
    char text[1024];
    vsnprintf(text, 1024, fmt, arg);
    logger(level) << text;
}

std::ostream &logger(int log_level) {
    if (!log_stream) log_stream = &std::cout;

    char tmstr[16];
    time_t now = time(nullptr);
    const char *levelstr[] = {"[DEBUG]", "[INFO] ", "[WARN] ", "[ERROR]"};
    strftime(tmstr, 16, "%y%m%d %H%M%S", localtime(&now));

    *log_stream << tmstr << ' ' << levelstr[log_level] << ": ";
    return *log_stream;
}

}
