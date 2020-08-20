#pragma once

#include "libretro.h"

#include <string>
#include <cstdint>

namespace libretro {
extern struct retro_vfs_interface vfs_interface;
}

#ifdef _WIN32
#define PATH_SEPARATOR_CHAR "\\"
#else
#define PATH_SEPARATOR_CHAR "/"
#endif

uint64_t get_ticks_usec();
int util_mkdir(const std::string &path, bool recursive = false);
bool util_file_exists(const std::string &path);
uint32_t utf8_to_ucs4(const char *&text);

template<typename T>
inline bool util_read_file(const std::string &filename, T &data) {
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
inline bool util_write_file(const std::string &filename, T &data) {
    auto *handle = libretro::vfs_interface.open(filename.c_str(), RETRO_VFS_FILE_ACCESS_WRITE, 0);
    if (handle == nullptr) return false;
    libretro::vfs_interface.write(handle, &data[0], data.size());
    libretro::vfs_interface.close(handle);
    return true;
}
