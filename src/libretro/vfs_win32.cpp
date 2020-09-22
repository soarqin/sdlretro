#ifdef VFS_WIN32

#include "util.h"

#include <libretro.h>

#include <windows.h>
#include <shlwapi.h>

#include <sys/types.h>
#include <sys/stat.h>

struct retro_vfs_file_handle {
    char filename[MAX_PATH * 3 + 1];
    wchar_t filenamew[MAX_PATH + 1];
    HANDLE file_handle;
};

struct retro_vfs_dir_handle {
    HANDLE dir_handle;
    WIN32_FIND_DATAW find_data;
    wchar_t dirnamew[MAX_PATH + 1];
    bool hidden;
    bool next;
};

namespace libretro {

inline bool FileNameUCSToUTF8(const wchar_t *filenamew, char filename[MAX_PATH * 3 + 1]) {
    return WideCharToMultiByte(CP_UTF8, 0, filenamew, -1, filename, MAX_PATH * 3 + 1, nullptr, 0) > 0;
}

inline bool FileNameUTF8ToUCS(const char *filename, wchar_t filenamew[MAX_PATH + 1]) {
    return MultiByteToWideChar(CP_UTF8, 0, filename, -1, filenamew, MAX_PATH + 1) > 0;
}

const char *RETRO_CALLCONV win32_vfs_get_path(struct retro_vfs_file_handle *stream) {
    return stream->filename;
}

struct retro_vfs_file_handle *RETRO_CALLCONV win32_vfs_open(const char *path, unsigned mode, unsigned hints) {
    auto *ret = new retro_vfs_file_handle;
    lstrcpynA(ret->filename, path, MAX_PATH * 3 + 1);
    if (!FileNameUTF8ToUCS(ret->filename, ret->filenamew)) return nullptr;
    DWORD access = 0;
    DWORD share_mode = FILE_SHARE_READ|FILE_SHARE_WRITE;
    DWORD creation = 0;
    if (mode & RETRO_VFS_FILE_ACCESS_READ) {
        access |= GENERIC_READ;
        creation = OPEN_EXISTING;
    }
    if (mode & RETRO_VFS_FILE_ACCESS_WRITE) {
        access |= GENERIC_WRITE;
        share_mode &= ~FILE_SHARE_WRITE;
        if (mode & RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING)
            creation = OPEN_EXISTING;
        else
            creation = CREATE_ALWAYS;
    }
    ret->file_handle = CreateFileW(ret->filenamew, access, share_mode, nullptr, creation, 0, nullptr);
    if (ret->file_handle == nullptr || ret->file_handle == INVALID_HANDLE_VALUE) {
        delete ret;
        return nullptr;
    }
    return ret;
}

int RETRO_CALLCONV win32_vfs_close(struct retro_vfs_file_handle *stream) {
    if (!stream) return -1;
    CloseHandle(stream->file_handle);
    delete stream;
    return 0;
}

int64_t RETRO_CALLCONV win32_vfs_size(struct retro_vfs_file_handle *stream) {
    LARGE_INTEGER size;
    if (GetFileSizeEx(stream->file_handle, &size))
        return size.QuadPart;
    return -1;
}

int64_t RETRO_CALLCONV win32_vfs_truncate(struct retro_vfs_file_handle *stream, int64_t length) {
    LARGE_INTEGER li = {0};
    LARGE_INTEGER liNew = {0};
    SetFilePointerEx(stream->file_handle, li, &liNew, FILE_CURRENT);
    li.QuadPart = length;
    SetFilePointerEx(stream->file_handle, li, nullptr, FILE_BEGIN);
    auto res = SetEndOfFile(stream->file_handle);
    if (res && length < liNew.QuadPart) liNew.QuadPart = length;
    SetFilePointerEx(stream->file_handle, liNew, nullptr, FILE_BEGIN);
    return res ? 0 : -1;
}

int64_t RETRO_CALLCONV win32_vfs_tell(struct retro_vfs_file_handle *stream) {
    LARGE_INTEGER li = {0};
    LARGE_INTEGER liNew = {0};
    if (SetFilePointerEx(stream->file_handle, li, &liNew, FILE_CURRENT))
        return liNew.QuadPart;
    return -1;
}

int64_t win32_vfs_seek(struct retro_vfs_file_handle *stream, int64_t offset, int seek_position) {
    LARGE_INTEGER li = {0};
    LARGE_INTEGER liNew = {0};
    li.QuadPart = offset;
    if (SetFilePointerEx(stream->file_handle, li, &liNew, seek_position))
        return liNew.QuadPart;
    return -1;
}

int64_t RETRO_CALLCONV win32_vfs_read(struct retro_vfs_file_handle *stream, void *s, uint64_t len) {
    auto *buf = static_cast<uint8_t*>(s);
    int64_t res = 0;
    while (len > 0) {
        DWORD read_bytes;
        if (!ReadFile(stream->file_handle, buf, len > 0xFFFFFFFCULL ? 0xFFFFFFFCU : static_cast<DWORD>(len), &read_bytes, nullptr)) {
            break;
        }
        res += read_bytes;
        buf += read_bytes;
        len -= read_bytes;
    }
    return res ? res : -1;
}

int64_t RETRO_CALLCONV win32_vfs_write(struct retro_vfs_file_handle *stream, const void *s, uint64_t len) {
    const auto *buf = static_cast<const uint8_t*>(s);
    int64_t res = 0;
    while (len > 0) {
        DWORD written_bytes;
        if (!WriteFile(stream->file_handle, buf, len > 0xFFFFFFFCULL ? 0xFFFFFFFCU : static_cast<DWORD>(len), &written_bytes, nullptr)) {
            break;
        }
        res += written_bytes;
        buf += written_bytes;
        len -= written_bytes;
    }
    return res ? res : -1;
}

int RETRO_CALLCONV win32_vfs_flush(struct retro_vfs_file_handle *stream) {
    return FlushFileBuffers(stream->file_handle) ? 0 : -1;
}

int RETRO_CALLCONV win32_vfs_remove(const char *path) {
    wchar_t filenamew[MAX_PATH + 1];
    if (!FileNameUTF8ToUCS(path, filenamew)) return -1;
    return DeleteFileW(filenamew) ? 0 : -1;
}

int RETRO_CALLCONV win32_vfs_rename(const char *old_path, const char *new_path) {
    wchar_t old_filenamew[MAX_PATH + 1];
    wchar_t new_filenamew[MAX_PATH + 1];
    if (!FileNameUTF8ToUCS(old_path, old_filenamew)) return -1;
    if (!FileNameUTF8ToUCS(new_path, new_filenamew)) return -1;
    return MoveFileW(old_filenamew, new_filenamew) ? 0 : -1;
}

#ifndef S_ISCHR
#define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#endif
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

int RETRO_CALLCONV win32_vfs_stat(const char *path, int32_t *size) {
    wchar_t filenamew[MAX_PATH + 1];
    if (!FileNameUTF8ToUCS(path, filenamew)) return 0;
    struct _stat s;
    _wstat(filenamew, &s);
    if (size) *size = s.st_size;
    return RETRO_VFS_STAT_IS_VALID | (S_ISDIR(s.st_mode) ? RETRO_VFS_STAT_IS_DIRECTORY : 0) | (S_ISCHR(s.st_mode) ? RETRO_VFS_STAT_IS_CHARACTER_SPECIAL : 0);
}

int RETRO_CALLCONV win32_vfs_mkdir(const char *dir) {
    return util::mkdir(dir);
}

struct retro_vfs_dir_handle *RETRO_CALLCONV win32_vfs_opendir(const char *dir, bool include_hidden) {
    wchar_t dirnamew[MAX_PATH + 1];
    if (!FileNameUTF8ToUCS(dir, dirnamew)) return nullptr;
    auto *handle = new retro_vfs_dir_handle;
    PathAppendW(dirnamew, L"*");
    memset(&handle->find_data, 0, sizeof(handle->find_data));
    handle->dir_handle = FindFirstFileW(dirnamew, &handle->find_data);
    if (handle->dir_handle == nullptr || handle->dir_handle == INVALID_HANDLE_VALUE) {
        delete handle;
        return nullptr;
    }
    lstrcpynW(handle->dirnamew, dirnamew, MAX_PATH + 1);
    handle->hidden = include_hidden;
    handle->next = false;
    return handle;
}

bool RETRO_CALLCONV win32_vfs_readdir(struct retro_vfs_dir_handle *dirstream) {
    for (;;) {
        if (dirstream->next) {
            if (!FindNextFileW(dirstream->dir_handle, &dirstream->find_data)) return false;
        }
        dirstream->next = true;
        if (!dirstream->hidden && (dirstream->find_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0) continue;
        return true;
    }
}

const char *RETRO_CALLCONV win32_vfs_dirent_get_name(struct retro_vfs_dir_handle *dirstream) {
    static char name[MAX_PATH * 3 + 1];
    if (!FileNameUCSToUTF8(dirstream->find_data.cFileName, name)) return nullptr;
    return name;
}

bool RETRO_CALLCONV win32_vfs_dirent_is_dir(struct retro_vfs_dir_handle *dirstream) {
    return (dirstream->find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

int RETRO_CALLCONV win32_vfs_closedir(struct retro_vfs_dir_handle *dirstream) {
    bool res = FindClose(dirstream->dir_handle);
    delete dirstream;
    return res ? 0 : -1;
}

struct retro_vfs_interface vfs_interface = {
    /* VFS API v1 */
    win32_vfs_get_path,
    win32_vfs_open,
    win32_vfs_close,
    win32_vfs_size,
    win32_vfs_tell,
    win32_vfs_seek,
    win32_vfs_read,
    win32_vfs_write,
    win32_vfs_flush,
    win32_vfs_remove,
    win32_vfs_rename,
    /* VFS API v2 */
    win32_vfs_truncate,
    /* VFS API v3 */
    win32_vfs_stat,
    win32_vfs_mkdir,
    win32_vfs_opendir,
    win32_vfs_readdir,
    win32_vfs_dirent_get_name,
    win32_vfs_dirent_is_dir,
    win32_vfs_closedir,
};

}

#endif
