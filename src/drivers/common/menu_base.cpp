#include "menu_base.h"

#include "driver_base.h"
#include "input_base.h"
#include "video_base.h"

#include <libretro.h>

#include <unistd.h>

namespace drivers {

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
    if (top_index + page_size > selected + 1) {
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
    if (top_index + page_size > selected) {
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
    if (top_index + page_size > selected + 1) {
        top_index = selected >= page_size ? selected + 1 - page_size : 0;
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
        page_up();
        return true;
    }
    if (states & (1<<RETRO_DEVICE_ID_JOYPAD_RIGHT)) {
        page_down();
        return true;
    }
    if (states & (1<<RETRO_DEVICE_ID_JOYPAD_L)) {
        move_first();
        return true;
    }
    if (states & (1<<RETRO_DEVICE_ID_JOYPAD_R)) {
        move_last();
        return true;
    }
    if (states & (1<<RETRO_DEVICE_ID_JOYPAD_A)) {
        ok_pressed = true;
        running = false;
        return false;
    }
    if (states & (1<<RETRO_DEVICE_ID_JOYPAD_B)) {
        ok_pressed = false;
        running = false;
        return false;
    }
    return false;
}

}
