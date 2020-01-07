#include "ui_menu.h"

#include "sdl1_menu.h"
#include "video_base.h"
#include "driver_base.h"

namespace gui {

int ui_menu::select_core_menu(const std::vector<const libretro::core_info *> &core_list) {
    drivers::sdl1_menu menu(driver, true);

    return 0;
}

}
