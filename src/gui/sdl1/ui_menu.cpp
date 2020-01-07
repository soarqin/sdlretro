#include "ui_menu.h"

#include "cfg.h"

#include "sdl1_menu.h"
#include "video_base.h"
#include "driver_base.h"

#include "core_manager.h"

namespace gui {

int ui_menu::select_core_menu(const std::vector<const libretro::core_info *> &core_list) {
    drivers::sdl1_menu menu(driver, true);

    menu.set_title("[select core to use]");

    std::vector<drivers::menu_item> items;
    for (const auto *core: core_list) {
        drivers::menu_item mi = {core->name + ' ' + core->version };
        items.emplace_back(mi);
    }
    menu.set_items(items);
    menu.set_rect(50, 50, DEFAULT_WIDTH - 100, DEFAULT_HEIGHT - 100);
    if (!menu.enter_menu_loop()) return -1;
    return menu.get_selected();
}

}
