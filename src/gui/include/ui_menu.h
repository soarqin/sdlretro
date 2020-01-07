#pragma once

#include <vector>
#include <memory>
#include <cstdint>

namespace libretro {
struct core_info;
}

namespace drivers {
class driver_base;
}

namespace gui {

class ui_menu {
public:
    inline explicit ui_menu(const std::shared_ptr<drivers::driver_base> &driver): driver(driver) {}
    virtual ~ui_menu() = default;

    /* select core from menu, return -1 if `cancel` is pressed */
    int select_core_menu(const std::vector<const libretro::core_info*> &core_list);

private:
    std::shared_ptr<drivers::driver_base> driver;
};

}
