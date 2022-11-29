#pragma once

#include "libretro.h"

#include <string>
#include <cstdint>

#ifdef _MSC_VER
#include <windows.h>
#define strcasecmp _stricmp
#define usleep(n) Sleep((n) / 1000)
#else
#include <unistd.h>
#endif

namespace libretro {
extern struct retro_vfs_interface vfs_interface;
}

#ifdef _WIN32
#define PATH_SEPARATOR_CHAR "\\"
#define DYNLIB_EXTENSION "dll"
#else
#define PATH_SEPARATOR_CHAR "/"
#define DYNLIB_EXTENSION "so"
#endif

namespace helper {

uint64_t get_ticks_usec();
uint64_t get_ticks_usec_cache();
uint64_t get_ticks_perfcounter();
int mkdir(const std::string &path, bool recursive = false);
bool file_exists(const std::string &path);
uint32_t utf8_to_ucs4(const char *&text);

template<typename T>
inline bool read_file(const std::string &filename, T &data) {
    auto *handle = libretro::vfs_interface.open(filename.c_str(), RETRO_VFS_FILE_ACCESS_READ, 0);
    if (handle == nullptr) return false;
    auto sz = libretro::vfs_interface.size(handle);
    if (sz < 0) {
        libretro::vfs_interface.close(handle);
        return false;
    }
    data.resize(sz);
    libretro::vfs_interface.read(handle, &data[0], sz);
    libretro::vfs_interface.close(handle);
    return true;
}

template<typename T>
inline bool write_file(const std::string &filename, T &data) {
    auto *handle = libretro::vfs_interface.open(filename.c_str(), RETRO_VFS_FILE_ACCESS_WRITE, 0);
    if (handle == nullptr) return false;
    if (!data.empty()) {
        libretro::vfs_interface.write(handle, &data[0], sizeof(data[0]) * data.size());
    }
    libretro::vfs_interface.close(handle);
    return true;
}

}
