#pragma once

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void log_vprintf(unsigned level, const char *fmt, va_list arg);

#ifdef __cplusplus
}
#endif
