#include "logger.h"

#include <cstdio>
#include <ctime>

void log_vprintf(unsigned level, const char *fmt, va_list arg) {
    char text[1024];
    char tmstr[16];
    time_t now = time(0);
    const char *levelstr[] = {"[DEBUG]", "[INFO] ", "[WARN] ", "[ERROR]"};
    strftime(tmstr, 16, "%y%m%d %H%M%S", localtime(&now));
    vsnprintf(text, 1024, fmt, arg);
    fprintf(stderr, "%s %s: %s", tmstr, levelstr[level], text);
}
