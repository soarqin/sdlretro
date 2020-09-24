#include "sdl_menu.h"

#include "driver_base.h"
#include "video_base.h"

#include "i18n.h"

namespace gui {

enum {
    gap_between_key_and_value = 20,
    slider_width = 10,
};

void sdl_menu::enter() {
    auto *video = driver->get_video();
    auto font_size = video->get_font_size();
    line_height = font_size + font_size / 4 + line_spacing;
    int ww, wh;
    video->get_resolution(ww, wh);
    if (menu_width == 0) menu_width = ww - menu_x;
    if (menu_height == 0) menu_height = wh - menu_y;
    if (!title.empty()) menu_height = menu_height - line_height - font_size / 2;
    page_size = (menu_height + font_size / 4 + line_spacing) / line_height;

    int tw;
    int tt, tb;
    video->get_text_width_and_height(title.c_str(), tw, tt, tb);
    title_x = (menu_width - tw) / 2 + menu_x;

    int maxwidth = 0;
    top_most = 255;
    bot_most = -255;
    int maxvaluewidth = 0;
    for (auto &item: items) {
        video->get_text_width_and_height(item.text.c_str(), tw, tt, tb);
        if (tw > maxwidth) maxwidth = tw;
        if (tt < top_most) top_most = tt;
        if (tb > bot_most) bot_most = tb;
        switch (item.type) {
        case menu_boolean:
            video->get_text_width_and_height("on"_i18n, tw, tt, tb);
            if (tw > maxvaluewidth) maxvaluewidth = tw;
            if (tt < top_most) top_most = tt;
            if (tb > bot_most) bot_most = tb;
            break;
        case menu_values:
            for (auto &val: item.values) {
                video->get_text_width_and_height(val.c_str(), tw, tt, tb);
                if (tw > maxvaluewidth) maxvaluewidth = tw;
                if (tt < top_most) top_most = tt;
                if (tb > bot_most) bot_most = tb;
            }
            break;
        case menu_input:
            video->get_text_width_and_height(std::string(20, 'a').c_str(), tw, tt, tb);
            if (tw > maxvaluewidth) maxvaluewidth = tw;
            if (tt < top_most) top_most = tt;
            if (tb > bot_most) bot_most = tb;
            break;
        default:
            break;
        }
    }
    if (item_width == 0 || item_width > maxwidth)
        item_width = maxwidth;
    int maxw = item_width + (maxvaluewidth > 0 ? (gap_between_key_and_value + maxvaluewidth) : 0);
    need_slider = page_size < items.size();
    if (need_slider) {
        maxw -= slider_width + 10;
    }
    if (maxw > menu_width) {
        key_x = menu_x;
        if (need_slider) {
            key_x += slider_width + 10;
        }
        if (maxvaluewidth > 0) {
            value_width = menu_width - gap_between_key_and_value - item_width;
        } else {
            value_width = 0;
        }
    } else {
        menu_x += (menu_width - maxw) / 2;
        key_x = menu_x;
        if (need_slider) {
            key_x += slider_width + 10;
        }
        value_width = maxvaluewidth > 0 ? maxvaluewidth : 0;
    }
    value_x = key_x + item_width + gap_between_key_and_value;
}

void sdl_menu::leave() {
    driver->get_video()->clear();
}

void sdl_menu::draw() {
    auto *video = driver->get_video();
    auto font_size = video->get_font_size();
    int x = key_x, y = menu_y + font_size;
    video->clear();
    video->predraw_menu();
    if (!title.empty()) {
        video->draw_text(title_x, y, title.c_str(), 0, true);
        y += line_height + font_size / 2;
    }
    int top_y = y - font_size;
    size_t end_index = top_index + page_size;
    auto item_size = items.size();
    if (end_index > item_size) end_index = item_size;
    for (size_t i = top_index; i < end_index; ++i) {
        auto &item = items[i];
        if (i == selected) {
            video->set_draw_color(0x60, 0x80, 0xA0, 0xFF);
            video->fill_rectangle(x - 3, y + top_most - 3, item_width + (value_width ? (gap_between_key_and_value + value_width) : 0) + 6, bot_most - top_most + 6);
        }
        video->draw_text(x, y, item.text.c_str(), item_width, true);
        switch (item.type) {
        case menu_boolean:
            video->draw_text(value_x, y, item.selected ? "on"_i18n : "off"_i18n, value_width, true);
            break;
        case menu_values:
            video->draw_text(value_x, y, item.values[item.selected].c_str(), value_width, true);
            break;
        case menu_input:
            video->draw_text(value_x, y, item.str.c_str(), value_width, true);
            break;
        default:
            break;
        }
        y += line_height;
    }
    if (need_slider) {
        int end_y = y - font_size;
        int top_pos = top_y + (end_y - top_y) * (int)top_index / (int)item_size;
        int end_pos = top_y + (end_y - top_y) * (int)end_index / (int)item_size;
        video->set_draw_color(0xC0, 0xC0, 0xC0, 0xFF);
        video->draw_rectangle(menu_x, top_y, slider_width, end_y - top_y);
        video->fill_rectangle(menu_x, top_pos, slider_width, end_pos - top_pos);
    }
    if (in_input_mode) {
        const char *dialog_text = "Press a key/button..."_i18n;
        int tw;
        int tt, tb;
        video->get_text_width_and_height(dialog_text, tw, tt, tb);

        int ww, wh;
        video->get_resolution(ww, wh);

        int dx = (ww - (int)tw) / 2;
        int dy = (wh - (tb - tt)) / 2 + font_size;

        video->set_draw_color(0x20, 0x20, 0x20, 0xC0);
        video->fill_rectangle(dx - 2, dy + tt - 2, (int)tw + 2, tb - tt + 4);
        video->draw_text(dx, dy, dialog_text, 0, false);
    }
    video->flip();
}

}
