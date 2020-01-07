#include <util.h>

#include <ctime>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

uint64_t get_ticks_usec() {
#ifndef CLOCK_MONOTONIC_COARSE
#define CLOCK_MONOTONIC_COARSE CLOCK_MONOTONIC
#endif
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    return ts.tv_sec*1000000ULL + ts.tv_nsec/1000ULL;
}

void util_mkdir(const char *path) {
#ifdef _WIN32
    wchar_t wpath[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, MAX_PATH);
    CreateDirectoryW(wpath, nullptr);
#else
    mkdir(path, 0755);
#endif
}
