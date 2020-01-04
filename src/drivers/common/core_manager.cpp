#include "core_manager.h"

#include "cfg.h"

#include <libretro.h>

#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#else
#include <dirent.h>
#endif

namespace drivers {

core_manager::core_manager(const std::string &static_root, const std::string &config_root) {
    g_cfg.set_filename(config_root + "/sdlretro.json");
    static_root_dir = static_root;
    config_root_dir = config_root;
    core_dirs.push_back(config_root + "/cores");
    if (static_root != config_root)
        core_dirs.push_back(static_root + "/cores");
    g_cfg.load();

    for (const auto &core_dir: core_dirs) {
#ifdef _WIN32
        wchar_t path[MAX_PATH] = {}, findpath[MAX_PATH];
        MultiByteToWideChar(CP_UTF8, 0, core_dir.c_str(), core_dir.length(), path, MAX_PATH);
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
                WideCharToMultiByte(CP_UTF8, 0, fullpath, lstrlenW(fullpath), filename, MAX_PATH*3, nullptr, nullptr);

                core_info coreinfo = {filename, info.library_name, info.library_version};
                std::string exts = info.valid_extensions;

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

            core_info coreinfo = {path, info.library_name, info.library_version};
            std::string exts = info.valid_extensions;

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

core_manager::~core_manager() {
    g_cfg.save();
}

std::vector<core_info> core_manager::match_cores_by_extension(const std::string &ext) {
    std::vector<core_info> core_list;
    for (const auto &c: cores) {
        for (const auto &extension: c.extensions) {
            if (strcasecmp(extension.c_str(), ext.c_str()) == 0) {
                core_list.push_back(c);
                break;
            }
        }
    }
    return core_list;
}

}
