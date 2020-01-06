#include "sdl1_menu.h"

#include "sdl1_video.h"
#include "sdl1_input.h"

#include "driver_base.h"

#include <SDL.h>

namespace drivers {

void sdl1_menu::enter() {
}

void sdl1_menu::leave() {
}

void sdl1_menu::draw() {
    int x = pos_x, y = pos_y;
    auto *video = static_cast<sdl1_video*>(driver->get_video());
    for (auto &item: items) {
        video->draw_text(x, y, item.text.c_str(), false, true);
        y += 12;
    }
}

}
