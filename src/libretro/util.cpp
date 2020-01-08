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
