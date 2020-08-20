#include "sdl_menu.h"

#include "driver_base.h"
#include "video_base.h"

namespace gui {

#ifdef GCW_ZERO
const int indicator_width = 8;
#else
const int indicator_width = 10;
#endif

void sdl_menu::enter() {
    auto *video = driver->get_video();
#ifdef GCW_ZERO
    line_height = 9 + line_spacing;
#else
    line_height = 18 + line_spacing;
#endif
    int ww, wh;
    video->get_resolution(ww, wh);
    if (menu_width == 0) menu_width = ww - menu_x;
    if (menu_height == 0) menu_height = wh - menu_y;
    if (!title.empty()) menu_height = menu_height - line_height - 4;
    page_size = (menu_height + line_spacing) / line_height;

    title_x = (menu_width - video->get_text_width(title.c_str())) / 2 + menu_x;

    int maxwidth = 0;
    int maxvaluewidth = 0;
    for (auto &item: items) {
        int w = video->get_text_width(item.text.c_str());
        if (w > maxwidth) maxwidth = w;
        switch (item.type) {
        case menu_boolean:
            w = video->get_text_width("yes");
            if (w > maxvaluewidth) maxvaluewidth = w;
            break;
        case menu_values:
            for (auto &val: item.values) {
                w = video->get_text_width(val.c_str());
                if (w > maxvaluewidth) maxvaluewidth = w;
            }
            break;
        default:
            break;
        }
    }
    if (item_width == 0 || item_width > maxwidth)
        item_width = maxwidth;
    auto maxw = indicator_width + item_width + (maxvaluewidth > 0 ? (20 + maxvaluewidth) : 0);
    if (maxw > menu_width) key_x = menu_x;
    else key_x = menu_x + (menu_width - maxw) / 2;
    value_x = key_x + indicator_width + item_width + 20;
    value_width = maxvaluewidth > 0 ? (menu_x + menu_width - value_x) : 0;
}

void sdl_menu::leave() {
    driver->get_video()->clear();
}

void sdl_menu::draw() {
    int x = key_x + indicator_width, y = menu_y;
    auto *video = driver->get_video();
    video->clear();
    video->predraw_menu();
    if (!title.empty()) {
        video->draw_text(title_x, y, title.c_str(), 0, true);
        y += line_height + 4;
    }
    size_t end_index = top_index + page_size;
    if (end_index > items.size()) end_index = items.size();
    for (size_t i = top_index; i < end_index; ++i) {
        if (i == selected) {
            video->draw_text(key_x, y, ">", 0, true);
        }
        auto &item = items[i];
        video->draw_text(x, y, item.text.c_str(), item_width, true);
        switch (item.type) {
        case menu_boolean:
            video->draw_text(value_x, y, item.selected ? "yes" : "no", value_width, true);
            break;
        case menu_values:
            video->draw_text(value_x, y, item.values[item.selected].c_str(), value_width, true);
            break;
        default:
            break;
        }
        y += line_height;
    }
    video->flip();
}

}
