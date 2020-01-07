#include "sdl1_menu.h"

#include "sdl1_video.h"
#include "sdl1_input.h"

#include "driver_base.h"

#include <SDL.h>

namespace drivers {

void sdl1_menu::enter() {
    auto *video = static_cast<sdl1_video*>(driver->get_video());
    line_height = (int)video->get_height() + line_spacing;
    if (menu_width == 0) menu_width = (int)video->get_width() - menu_x;
    if (menu_height == 0) menu_height = (int)video->get_height() - menu_y;
    page_size = (menu_height + line_spacing) / line_height;
}

void sdl1_menu::leave() {
}

void sdl1_menu::draw() {
    int x = menu_x, y = menu_y;
    auto *video = static_cast<sdl1_video*>(driver->get_video());
    size_t end_index = top_index + page_size;
    if (end_index > items.size()) end_index = items.size();
    for (size_t i = top_index; i < end_index; ++i) {
        auto &item = items[i];
        video->draw_text(x, y, item.text.c_str(), false, true);
        y += 12;
    }
}

}
