#include "core_manager.h"

#include <libretro.h>

#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#else
#include <dirent.h>
#include <dlfcn.h>
#endif
#include <cstring>

namespace libretro {

#ifdef _WIN32
#define PATH_SEPARATOR_CHAR "\\"
#else
#define PATH_SEPARATOR_CHAR "/"
#endif

core_manager::core_manager(const std::vector<std::string> &search_dirs) {
    core_dirs = search_dirs;
    for (auto &d: core_dirs) {
        d += PATH_SEPARATOR_CHAR "cores";
    }
    for (const auto &core_dir: core_dirs) {
#ifdef _WIN32
        wchar_t path[MAX_PATH] = {}, findpath[MAX_PATH];
        MultiByteToWideChar(CP_UTF8, 0, core_dir.c_str(), -1, path, MAX_PATH);
        lstrcpyW(findpath, path);
        PathAppendW(findpath, L"\\*.dll");
        WIN32_FIND_DATAW data = {};
        HANDLE hFile = FindFirstFileW(findpath, &data);
        if (hFile!=INVALID_HANDLE_VALUE) {
            do {
                wchar_t fullpath[MAX_PATH] = {};
                lstrcpyW(fullpath, path);
                PathAppendW(fullpath, data.cFileName);
                HMODULE mod = LoadLibraryW(fullpath);
                if (mod==nullptr) continue;
                typedef void (*sysinfo_t)(struct retro_system_info *);
                auto sysinfo = (sysinfo_t)GetProcAddress(mod, "retro_get_system_info");
                if (!sysinfo) {
                    FreeLibrary(mod);
                    continue;
                }
                retro_system_info info = {};
                sysinfo(&info);

                char filename[MAX_PATH*3] = {};
                WideCharToMultiByte(CP_UTF8, 0, fullpath, -1, filename, MAX_PATH*3, nullptr, nullptr);

                core_info coreinfo = {filename, info.library_name ? info.library_name : filename, info.library_version ? info.library_name : "unknown"};
                coreinfo.need_fullpath = info.need_fullpath;
                std::string exts = info.valid_extensions ? info.valid_extensions : "";

                FreeLibrary(mod);

                for (;;) {
                    auto pos = exts.find('|');
                    coreinfo.extensions.push_back(exts.substr(0, pos));
                    if (pos==std::string::npos) break;
                    exts = exts.substr(pos + 1);
                }
                cores.push_back(coreinfo);
            } while (FindNextFileW(hFile, &data));
            FindClose(hFile);
        }
#else
        DIR *d = opendir(core_dir.c_str());
        if (d == nullptr) return;
        dirent *ent;
        while ((ent = readdir(d)) != nullptr) {
            std::string path = core_dir + '/' + ent->d_name;
            void *dynlib = dlopen(path.c_str(), RTLD_LAZY);
            if (dynlib == nullptr) continue;

            typedef void (*sysinfo_t)(struct retro_system_info*);
            auto sysinfo = (sysinfo_t)dlsym(dynlib, "retro_get_system_info");
            if (!sysinfo) {
                dlclose(dynlib);
                continue;
            }
            retro_system_info info = {};
            sysinfo(&info);

            core_info coreinfo = {path, info.library_name ? info.library_name : filename, info.library_version ? info.library_name : "unknown"};
            coreinfo.need_fullpath = info.need_fullpath;
            std::string exts = info.valid_extensions ? info.valid_extensions : "";

            dlclose(dynlib);

            for (;;) {
                auto pos = exts.find('|');
                coreinfo.extensions.push_back(exts.substr(0, pos));
                if (pos == std::string::npos) break;
                exts = exts.substr(pos + 1);
            }
            cores.push_back(coreinfo);
        }
        closedir(d);
#endif
    }
}

std::vector<const core_info *> core_manager::match_cores_by_extension(const std::string &ext) {
    std::vector<const core_info *> core_list;
    for (const auto &c: cores) {
        for (const auto &extension: c.extensions) {
            if (strcasecmp(extension.c_str(), ext.c_str())==0) {
                core_list.push_back(&c);
                break;
            }
        }
    }
    return core_list;
}

}
