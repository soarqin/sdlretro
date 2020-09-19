#ifdef VFS_UNIX

#include "util.h"

#include <libretro.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <dirent.h>

#ifdef WIN32
#define fsync
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif

struct retro_vfs_file_handle {
    char filename[PATH_MAX + 1];
    int file_handle;
};

struct retro_vfs_dir_handle {
    char dirname[PATH_MAX + 1];
    DIR *dir;
    struct dirent *data;
    bool hidden;
};

namespace libretro {

const char *RETRO_CALLCONV unix_vfs_get_path(struct retro_vfs_file_handle *stream) {
    return stream->filename;
}

struct retro_vfs_file_handle *RETRO_CALLCONV unix_vfs_open(const char *path, unsigned mode, unsigned hints) {
    auto *ret = new retro_vfs_file_handle;
    int flag = O_BINARY;
    if (mode & (RETRO_VFS_FILE_ACCESS_READ | RETRO_VFS_FILE_ACCESS_WRITE)) {
        flag |= O_RDWR;
        if (!(mode & RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING)) {
            flag |= O_CREAT | O_TRUNC;
        }
    } else if (mode & RETRO_VFS_FILE_ACCESS_READ) {
        flag |= O_RDONLY;
    } else if (mode & RETRO_VFS_FILE_ACCESS_WRITE) {
        if (!(mode & RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING)) {
            flag |= O_WRONLY | O_CREAT | O_TRUNC;
        } else {
            flag |= O_RDWR;
        }
    }
    ret->file_handle = open(path, flag);
    if (ret->file_handle < 0) {
        delete ret;
        return nullptr;
    }
    snprintf(ret->filename, PATH_MAX + 1, "%s", path);
    return ret;
}

int RETRO_CALLCONV unix_vfs_close(struct retro_vfs_file_handle *stream) {
    if (!stream) return -1;
    close(stream->file_handle);
    delete stream;
    return 0;
}

int64_t RETRO_CALLCONV unix_vfs_size(struct retro_vfs_file_handle *stream) {
    auto off = lseek64(stream->file_handle, 0, SEEK_CUR);
    auto res = lseek64(stream->file_handle, 0, SEEK_END);
    lseek64(stream->file_handle, off, SEEK_SET);
    return res;
}

int64_t RETRO_CALLCONV unix_vfs_truncate(struct retro_vfs_file_handle *stream, int64_t length) {
    return ftruncate(stream->file_handle, length);
}

int64_t RETRO_CALLCONV unix_vfs_tell(struct retro_vfs_file_handle *stream) {
    return lseek64(stream->file_handle, 0, SEEK_CUR);
}

int64_t unix_vfs_seek(struct retro_vfs_file_handle *stream, int64_t offset, int seek_position) {
    return lseek64(stream->file_handle, offset, seek_position);
}

int64_t RETRO_CALLCONV unix_vfs_read(struct retro_vfs_file_handle *stream, void *s, uint64_t len) {
    auto *buf = static_cast<uint8_t*>(s);
    int64_t res = 0;
    while (len > 0) {
        auto read_bytes = read(stream->file_handle, buf, len > 0xFFFFFFFCULL ? 0xFFFFFFFCU : static_cast<unsigned>(len));
        if (read_bytes <= 0) break;
        res += read_bytes;
        buf += read_bytes;
        len -= read_bytes;
    }
    return res ? res : -1;
}

int64_t RETRO_CALLCONV unix_vfs_write(struct retro_vfs_file_handle *stream, const void *s, uint64_t len) {
    const auto *buf = static_cast<const uint8_t*>(s);
    int64_t res = 0;
    while (len > 0) {
        auto read_bytes = write(stream->file_handle, buf, len > 0xFFFFFFFCULL ? 0xFFFFFFFCU : static_cast<unsigned>(len));
        if (read_bytes <= 0) break;
        res += read_bytes;
        buf += read_bytes;
        len -= read_bytes;
    }
    return res ? res : -1;
}

int RETRO_CALLCONV unix_vfs_flush(struct retro_vfs_file_handle *stream) {
    return fsync(stream->file_handle);
}

int RETRO_CALLCONV unix_vfs_remove(const char *path) {
    return remove(path);
}

int RETRO_CALLCONV unix_vfs_rename(const char *old_path, const char *new_path) {
    return rename(old_path, new_path);
}

int RETRO_CALLCONV unix_vfs_stat(const char *path, int32_t *size) {
    struct stat s;
    if (stat(path, &s) != 0) return 0;
    if (size) *size = s.st_size;
    return RETRO_VFS_STAT_IS_VALID | (S_ISCHR(s.st_mode) ? RETRO_VFS_STAT_IS_DIRECTORY : 0) | (S_ISDIR(s.st_mode) ? RETRO_VFS_STAT_IS_CHARACTER_SPECIAL : 0);
}

int RETRO_CALLCONV unix_vfs_mkdir(const char *dir) {
    return util::mkdir(dir);
}

struct retro_vfs_dir_handle *RETRO_CALLCONV unix_vfs_opendir(const char *dir, bool include_hidden) {
    auto *handle = new retro_vfs_dir_handle;
    handle->dir = opendir(dir);
    if (handle->dir == nullptr) {
        delete handle;
        return nullptr;
    }
    snprintf(handle->dirname, PATH_MAX + 1, "%s", dir);
    handle->hidden = include_hidden;
    return handle;
}

bool RETRO_CALLCONV unix_vfs_readdir(struct retro_vfs_dir_handle *dirstream) {
    for (;;) {
        dirstream->data = readdir(dirstream->dir);
        if (dirstream->data == nullptr) return false;
        /* ignore . and .. , as well as hidden files */
        if (dirstream->data->d_name[0] == '.' &&
            (dirstream->data->d_name[1] == 0
                || (dirstream->data->d_name[1] == '.' && dirstream->data->d_name[2] == 0)
                || !dirstream->hidden)) continue;
        return true;
    }
}

const char *RETRO_CALLCONV unix_vfs_dirent_get_name(struct retro_vfs_dir_handle *dirstream) {
    return dirstream->data->d_name;
}

bool RETRO_CALLCONV unix_vfs_dirent_is_dir(struct retro_vfs_dir_handle *dirstream) {
    char path[PATH_MAX + 1];
    snprintf(path, PATH_MAX + 1, "%s/%s", dirstream->dirname, dirstream->data->d_name);
    struct stat s;
    if (stat(path, &s) != 0) return false;
    return S_ISDIR(s.st_mode);
}

int RETRO_CALLCONV unix_vfs_closedir(struct retro_vfs_dir_handle *dirstream) {
    auto res = closedir(dirstream->dir);
    delete dirstream;
    return res;
}

struct retro_vfs_interface vfs_interface = {
    /* VFS API v1 */
    unix_vfs_get_path,
    unix_vfs_open,
    unix_vfs_close,
    unix_vfs_size,
    unix_vfs_tell,
    unix_vfs_seek,
    unix_vfs_read,
    unix_vfs_write,
    unix_vfs_flush,
    unix_vfs_remove,
    unix_vfs_rename,
    /* VFS API v2 */
    unix_vfs_truncate,
    /* VFS API v3 */
    unix_vfs_stat,
    unix_vfs_mkdir,
    unix_vfs_opendir,
    unix_vfs_readdir,
    unix_vfs_dirent_get_name,
    unix_vfs_dirent_is_dir,
    unix_vfs_closedir,
};

}

#endif
