#pragma once

#include <string>
#include <vector>

namespace libretro {

struct core_info {
    std::string filepath;
    std::string name;
    std::string version;
    std::vector<std::string> extensions;
    bool need_fullpath;
};

class core_manager final {
public:
    explicit core_manager();
    ~core_manager() = default;

    /* match libretro cores by rom file extention */
    std::vector<const core_info*> match_cores_by_extension(const std::string &ext);

private:
    std::vector<std::string> core_dirs;
    std::vector<core_info> cores;
};

}
