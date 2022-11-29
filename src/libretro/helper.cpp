#include <helper.h>

#include <ctime>

#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#else
#include <sys/stat.h>
#include <errno.h>
#endif

namespace helper {

static uint64_t ticks_usec_cache = 0ULL;

uint64_t get_ticks_usec() {
#ifdef _MSC_VER
    LARGE_INTEGER counter, freq;
    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&freq);
    ticks_usec_cache = counter.QuadPart / (freq.QuadPart / 1000000ULL);
#else
#ifndef CLOCK_MONOTONIC_COARSE
#define CLOCK_MONOTONIC_COARSE CLOCK_MONOTONIC
#endif
    timespec ts = {};
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    ticks_usec_cache = ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;
#endif
    return ticks_usec_cache;
}

uint64_t get_ticks_usec_cache() {
    return ticks_usec_cache;
}

uint64_t get_ticks_perfcounter() {
#ifdef _MSC_VER
    LARGE_INTEGER counter, freq;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart;
#else
    timespec ts = {};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
#endif
}

#ifdef _WIN32
static inline int mkdir_unicode(const wchar_t *wpath, bool recursive) {
    if (recursive) {
        wchar_t n[MAX_PATH];
        lstrcpyW(n, wpath);
        PathRemoveFileSpecW(n);
        if (!PathIsDirectoryW(n)) {
            int ret = mkdir_unicode(n, recursive);
            if (ret != 0) return ret;
        }
    }
    if (CreateDirectoryW(wpath, nullptr)) return 0;
    if (GetLastError() == ERROR_ALREADY_EXISTS) return -2;
    return -1;
}
#endif

int mkdir(const std::string &path, bool recursive) {
#ifdef _WIN32
    wchar_t wpath[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, wpath, MAX_PATH);
    return mkdir_unicode(wpath, recursive);
#else
    if (recursive) {
        auto pos = path.find_last_of('/');
        if (pos != std::string::npos) {
            auto parent = path.substr(0, pos);
            struct stat s = {};
            if (stat(parent.c_str(), &s) == -1) {
                int ret = mkdir(path.substr(0, pos), true);
                if (ret != 0) return ret;
            } else if (!S_ISDIR(s.st_mode)) {
                return -1;
            }
        }
    }
    if (::mkdir(path.c_str(), 0755) == 0) return 0;
    if (errno == EEXIST) return -2;
    return -1;
#endif
}

bool file_exists(const std::string &path) {
#ifdef _WIN32
    wchar_t wpath[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, wpath, MAX_PATH);
    DWORD dwAttrib = GetFileAttributesW(wpath);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    return access(path.c_str(), F_OK) != -1;
#endif
}

/* UTF-8 to UCS-4 */
uint32_t utf8_to_ucs4(const char *&text) {
    auto c = static_cast<uint8_t>(*text);
    if (c < 0x80) {
        uint16_t ch = c;
        ++text;
        return ch;
    } else if (c < 0xC0) {
        ++text;
        return 0;
    } else if (c < 0xE0) {
        uint16_t ch = (c & 0x1Fu) << 6u;
        c = static_cast<uint8_t>(*++text);
        if (c == 0) return 0;
        ch |= c & 0x3Fu;
        ++text;
        return ch;
    } else if (c < 0xF0) {
        uint16_t ch = (c & 0x0Fu) << 12u;
        c = static_cast<uint8_t>(*++text);
        if (c == 0) return 0;
        ch |= (c & 0x3Fu) << 6u;
        c = static_cast<uint8_t>(*++text);
        if (c == 0) return 0;
        ch |= c & 0x3Fu;
        ++text;
        return ch;
    } else if (c < 0xF8) {
        uint16_t ch = (c & 0x07u) << 18u;
        c = static_cast<uint8_t>(*++text);
        if (c == 0) return 0;
        ch |= (c & 0x3Fu) << 12u;
        c = static_cast<uint8_t>(*++text);
        if (c == 0) return 0;
        ch |= (c & 0x3Fu) << 6u;
        c = static_cast<uint8_t>(*++text);
        if (c == 0) return 0;
        ch |= c & 0x3Fu;
        ++text;
        return ch;
    } else if (c < 0xFC) {
        uint16_t ch = (c & 0x03u) << 24u;
        c = static_cast<uint8_t>(*++text);
        if (c == 0) return 0;
        ch |= (c & 0x3Fu) << 18u;
        c = static_cast<uint8_t>(*++text);
        if (c == 0) return 0;
        ch |= (c & 0x3Fu) << 12u;
        c = static_cast<uint8_t>(*++text);
        if (c == 0) return 0;
        ch |= (c & 0x3Fu) << 6u;
        c = static_cast<uint8_t>(*++text);
        if (c == 0) return 0;
        ch |= c & 0x3Fu;
        ++text;
        return ch;
    } else if (c < 0xFE) {
        uint16_t ch = (c & 0x03u) << 30u;
        c = static_cast<uint8_t>(*++text);
        if (c == 0) return 0;
        ch |= (c & 0x3Fu) << 24u;
        c = static_cast<uint8_t>(*++text);
        if (c == 0) return 0;
        ch |= (c & 0x3Fu) << 18u;
        c = static_cast<uint8_t>(*++text);
        if (c == 0) return 0;
        ch |= (c & 0x3Fu) << 12u;
        c = static_cast<uint8_t>(*++text);
        if (c == 0) return 0;
        ch |= (c & 0x3Fu) << 6u;
        c = static_cast<uint8_t>(*++text);
        if (c == 0) return 0;
        ch |= c & 0x3Fu;
        ++text;
        return ch;
    }
    ++text;
    return 0;
}

}
