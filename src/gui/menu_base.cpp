#include "menu_base.h"

#include "driver_base.h"
#include "input_base.h"
#include "video_base.h"

#include <libretro.h>

#include <unistd.h>

namespace gui {

menu_base::menu_base(std::shared_ptr<drivers::driver_base> d, bool t) : driver(std::move(d)), topmenu(t) {
}

bool menu_base::enter_menu_loop() {
    auto *input = driver->get_input();
    if (topmenu) {
        input->clear_menu_button_desc();

        input->set_input_mode(drivers::input_base::mode_menu);
        input->add_button_desc(0xFF, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "UP");
        input->add_button_desc(0xFF, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "DOWN");
        input->add_button_desc(0xFF, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "LEFT");
        input->add_button_desc(0xFF, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "RIGHT");
        input->add_button_desc(0xFF, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "L1");
        input->add_button_desc(0xFF, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "R1");
        input->add_button_desc(0xFF, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2, "L2");
        input->add_button_desc(0xFF, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2, "R2");
        input->add_button_desc(0xFF, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A");
        input->add_button_desc(0xFF, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B");

        driver->get_video()->enter_menu();
    }

    do {
        driver->process_events();
        input->input_poll();
        usleep(50000);
    } while(input->get_menu_pad_states() != 0);
    enter();
    running = true;
    draw();
    while (running) {
        usleep(50000);
        if (driver->process_events()) {
            running = false;
            ok_pressed = false;
            break;
        }
        if (poll_input()) {
            draw();
            usleep(100000);
        }
    }
    leave();

    if (topmenu) {
        driver->get_video()->leave_menu();
        driver->get_input()->set_input_mode(drivers::input_base::mode_game);
    }
    leave_menu_loop();
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
    if (in_input_mode) {
        uint64_t id;
        if ((id = input->get_last_input()) == 0) {
            return false;
        }

        std::string device_name, name;
        input->get_input_name(id, device_name, name);

        auto &item = items[selected];
        if (item.type == menu_input) {
            item.selected = id;
            item.str = device_name.empty() ? name : (name + " (" + device_name + ')');
            if (item.callback) {
                item.callback(item);
            }
        }

        in_input_mode = false;
        input->clear_last_input();
        input->set_input_mode(drivers::input_base::mode_menu);
        return true;
    }
    auto states = input->get_menu_pad_states();
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
        switch (item.type) {
        case menu_static:
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
        case menu_input:
            do {
                driver->process_events();
                input->input_poll();
                usleep(50000);
            } while(input->get_menu_pad_states() != 0);
            input->clear_last_input();
            input->set_input_mode(drivers::input_base::mode_input);
            in_input_mode = true;
            return true;
        default:
            break;
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
