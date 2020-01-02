#pragma once

#include <string>
#include <vector>

namespace drivers {

class driver_base;

struct core_info {
    std::string filepath;
    std::string name;
    std::string version;
    std::vector<std::string> extensions;
};

class core_manager {
public:
    explicit core_manager(const std::string &dir);

    /* match libretro cores by rom file extention */
    std::vector<core_info> match_cores_by_extension(const std::string &ext);

private:
    std::string core_dir;
    std::vector<core_info> cores;
};

}
