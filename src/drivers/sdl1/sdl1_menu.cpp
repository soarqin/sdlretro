#include "sdl1_menu.h"

#include "sdl1_video.h"
#include "sdl1_input.h"

#include "driver_base.h"

#include <SDL.h>

namespace drivers {

void sdl1_menu::enter() {
    auto *video = static_cast<sdl1_video*>(driver->get_video());
#ifdef GCW_ZERO
    line_height = 9 + line_spacing;
#else
    line_height = 18 + line_spacing;
#endif
    if (menu_width == 0) menu_width = (int)video->get_width() - menu_x;
    if (menu_height == 0) menu_height = (int)video->get_height() - menu_y;
    if (!title.empty()) menu_height = menu_height - line_height - 4;
    page_size = (menu_height + line_spacing) / line_height;
}

void sdl1_menu::leave() {
}

void sdl1_menu::draw() {
#ifdef GCW_ZERO
    const int indent = 8;
#else
    const int indent = 10;
#endif
    int x = menu_x + indent, y = menu_y;
    auto *video = static_cast<sdl1_video*>(driver->get_video());
    video->lock();
    video->clear();
    if (!title.empty()) {
        video->draw_text(menu_x, y, title.c_str(), false, true);
        y += line_height + 4;
    }
    size_t end_index = top_index + page_size;
    if (end_index > items.size()) end_index = items.size();
    for (size_t i = top_index; i < end_index; ++i) {
        if (i == selected) {
            video->draw_text(menu_x, y, ">", false, true);
        }
        auto &item = items[i];
        video->draw_text(x, y, item.text.c_str(), false, true);
        y += line_height;
    }
    video->unlock();
    video->flip();
}

}
