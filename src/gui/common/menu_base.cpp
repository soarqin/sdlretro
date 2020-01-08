#include "menu_base.h"

#include "driver_base.h"
#include "input_base.h"
#include "video_base.h"

#include <libretro.h>

#include <unistd.h>

namespace gui {

bool menu_base::enter_menu_loop() {
    if (topmenu) {
        driver->get_video()->enter_menu();
    }

    enter();
    running = true;
    draw();
    while (running) {
        usleep(75000);
        if (driver->process_events()) {
            running = false;
            ok_pressed = false;
            break;
        }
        if (poll_input()) {
            draw();
            usleep(75000);
        }
    }
    leave();

    if (topmenu) {
        driver->get_video()->leave_menu();
    }
    return ok_pressed;
}

void menu_base::leave_menu_loop() {
    running = false;
}

void menu_base::move_up() {
    if (selected == 0) {
        move_last();
        return;
    }
    selected--;
    if (selected < top_index + 1) {
        top_index = selected ? selected - 1 : selected;
    }
}

void menu_base::move_down() {
    size_t sz = items.size();
    if (++selected >= sz) {
        move_first();
        return;
    }
    auto page_size = page_count();
    if (top_index + page_size <= selected + 1) {
        top_index = selected + 1 >= page_size ? (selected + 1 == sz ? selected + 1 - page_size : selected + 2 - page_size) : 0;
    }
}

void menu_base::page_up() {
    if (selected == 0) return;
    auto page_size = page_count();
    selected = selected > page_size ? selected - page_size : 0;
    if (selected < top_index + 1) {
        top_index = selected ? selected - 1 : selected;
    }
}

void menu_base::page_down() {
    size_t sz = items.size();
    if (selected + 1 >= sz) return;
    auto page_size = page_count();
    if (selected + page_size >= sz) selected = sz - 1;
    if (top_index + page_size <= selected + 1) {
        top_index = selected + 1 >= page_size ? (selected + 1 == sz ? selected + 1 - page_size : selected + 2 - page_size) : 0;
    }
}

void menu_base::move_first() {
    selected = 0;
    if (selected < top_index + 1) {
        top_index = selected ? selected - 1 : selected;
    }
}

void menu_base::move_last() {
    if (items.empty()) return;
    selected = items.size() - 1;
    auto page_size = page_count();
    if (top_index + page_size <= selected + 1) {
        top_index = selected >= page_size ? selected + 1 - page_size : 0;
    }
}

void menu_base::value_dec() {
    auto &item = items[selected];
    switch (item.type) {
    case menu_boolean:
        item.selected ^= 1U;
        if (item.callback) item.callback(item);
        else if (item.data) *(bool*)item.data = item.selected;
        break;
    case menu_values:
        if (item.selected == 0) item.selected = item.values.size() - 1;
        else item.selected--;
        if (item.callback) item.callback(item);
        else if (item.data) *(uint32_t*)item.data = item.selected;
        break;
    default:
        break;
    }
}

void menu_base::value_inc() {
    auto &item = items[selected];
    switch (item.type) {
    case menu_boolean:
        item.selected ^= 1U;
        if (item.callback) item.callback(item);
        else if (item.data) *(bool*)item.data = item.selected;
        break;
    case menu_values:
        if (++item.selected >= item.values.size())
            item.selected = 0;
        if (item.callback) item.callback(item);
        else if (item.data) *(uint32_t*)item.data = item.selected;
        break;
    default:
        break;
    }
}

bool menu_base::poll_input() {
    auto *input = driver->get_input();
    input->input_poll();
    auto states = input->get_pad_states(0);
    if (states & (1<<RETRO_DEVICE_ID_JOYPAD_UP)) {
        move_up();
        return true;
    }
    if (states & (1<<RETRO_DEVICE_ID_JOYPAD_DOWN)) {
        move_down();
        return true;
    }
    if (states & (1<<RETRO_DEVICE_ID_JOYPAD_LEFT)) {
        value_dec();
        return true;
    }
    if (states & (1<<RETRO_DEVICE_ID_JOYPAD_RIGHT)) {
        value_inc();
        return true;
    }
    if (states & (1<<RETRO_DEVICE_ID_JOYPAD_L)) {
        page_up();
        return true;
    }
    if (states & (1<<RETRO_DEVICE_ID_JOYPAD_R)) {
        page_down();
        return true;
    }
    if (states & (1<<RETRO_DEVICE_ID_JOYPAD_L2)) {
        move_first();
        return true;
    }
    if (states & (1<<RETRO_DEVICE_ID_JOYPAD_R2)) {
        move_last();
        return true;
    }
    if (states & (1<<RETRO_DEVICE_ID_JOYPAD_A)) {
        auto &item = items[selected];
        if (item.type == menu_static) {
            if (item.callback) {
                if (item.callback(item)) {
                    running = false;
                    return false;
                }
                return true;
            }
            ok_pressed = true;
            running = false;
            return false;
        }
        /* TODO: handle menu_path */
    }
    if (states & (1<<RETRO_DEVICE_ID_JOYPAD_B)) {
        ok_pressed = false;
        running = false;
        return false;
    }
    return false;
}

}
