#pragma once

#include <utility>
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

struct menu_item;
class menu_base;

class ui_menu {
public:
    explicit ui_menu(std::shared_ptr<drivers::driver_base> drv);
    virtual ~ui_menu() = default;

    /* select core from menu, return -1 if `cancel` is pressed */
    int select_core_menu(const std::vector<const libretro::core_info*> &core_list);

    /* enter ingame menu */
    void in_game_menu();

protected:
    bool global_settings_menu(menu_base *parent);
    bool core_settings_menu(menu_base *parent);
    bool input_settings_menu(menu_base *parent);
    bool language_settings_menu(menu_base *parent);

private:
    std::shared_ptr<drivers::driver_base> driver;
};

}
