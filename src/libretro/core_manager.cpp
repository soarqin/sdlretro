#include "core_manager.h"

#include "cfg.h"

#include <util.h>

#include "dlfcn_compat.h"

#include <cstring>

namespace libretro {

core_manager::core_manager() {
    g_cfg.get_core_dirs(core_dirs);
    for (const auto &core_dir: core_dirs) {
        auto *dir = vfs_interface.opendir(core_dir.c_str(), false);
        if (!dir) continue;
        while (vfs_interface.readdir(dir)) {
            const char *name = vfs_interface.dirent_get_name(dir);
            std::string filename = core_dir;
            filename += PATH_SEPARATOR_CHAR;
            filename += name;

            void *dynlib = dlopen(filename.c_str(), RTLD_LAZY);
            if (dynlib == nullptr) continue;

            typedef void (*sysinfo_t)(struct retro_system_info*);
            auto sysinfo = (sysinfo_t)dlsym(dynlib, "retro_get_system_info");
            if (!sysinfo) {
                dlclose(dynlib);
                continue;
            }
            retro_system_info info = {};
            sysinfo(&info);

            core_info coreinfo = {filename, info.library_name ? info.library_name : name, info.library_version ? info.library_version : "unknown"};
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
        vfs_interface.closedir(dir);
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
