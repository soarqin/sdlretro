#pragma once

#include "driver_base.h"

#include <string>
#include <vector>

namespace drivers {

struct core_info {
    std::string filepath;
    std::string name;
    std::string version;
    std::vector<std::string> extensions;
};

class core_manager final {
public:
    explicit core_manager(const std::string &static_root, const std::string &config_root);
    ~core_manager();

    /* match libretro cores by rom file extention */
    std::vector<core_info> match_cores_by_extension(const std::string &ext);

    template<class T>
    inline std::unique_ptr<driver_base> load_core(const std::string &path) {
        driver_base *c = new(std::nothrow) T;
        if (c == nullptr) return nullptr;
        c->set_dirs(static_root_dir, config_root_dir);
        if (!c->load(path)) {
            delete c;
            return nullptr;
        }
        return std::unique_ptr<driver_base>(c);
    }


private:
    std::string static_root_dir;
    std::string config_root_dir;
    std::vector<std::string> core_dirs;
    std::vector<core_info> cores;
};

}
