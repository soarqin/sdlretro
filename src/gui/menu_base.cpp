#include "menu_base.h"

#include "driver_base.h"
#include "input_base.h"
#include "video_base.h"

#include <libretro.h>

#ifdef _MSC_VER
#include <windows.h>
#define usleep(n) Sleep((n) / 1000)
#else
#include <unistd.h>
#endif

namespace gui {

menu_base::menu_base(std::shared_ptr<drivers::driver_base> d, menu_base *p, std::function<void(menu_base&)> init_func) : driver(std::move(d)), parent(p), init_fn(std::move(init_func)) {
}

bool menu_base::enter_menu_loop(size_t sel) {
    auto *input = driver->get_input();
    if (!parent) {
        driver->get_video()->enter_menu();
    }

    do {
        force_refreshing = false;
        if (init_fn)
            init_fn(*this);
        running = true;
        do {
            if (driver->process_events()) {
                ok_pressed = false;
                leave_menu_loop();
                driver->shutdown();
                break;
            }
            input->input_poll();
            usleep(50000);
        } while (input->input_state(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK) != 0);
        if (!running) {
            break;
        }
        enter();
        set_selected(sel);
        draw();
        while (running && !force_refreshing) {
            usleep(50000);
            if (driver->process_events()) {
                ok_pressed = false;
                leave_menu_loop();
                driver->shutdown();
                break;
            }
            if (poll_input() && running && !force_refreshing) {
                draw();
                usleep(100000);
            }
        }
        sel = selected;
        leave();
    } while (force_refreshing);

    if (!parent) {
        driver->get_video()->leave_menu();
        input->set_input_mode(drivers::input_base::mode_game);
    }
    running = false;
    return ok_pressed;
}

void menu_base::leave_menu_loop() {
    running = false;
    if (parent) {
        parent->leave_menu_loop();
    }
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
    selected = selected + page_size >= sz ? (sz - 1) : (selected + page_size);
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

void menu_base::set_selected(size_t sel) {
    if (selected == sel) return;
    size_t sz = items.size();
    if (sel >= sz) {
        move_first();
        return;
    }
    selected = sel;
    auto page_size = page_count();
    if (selected < top_index + 1) {
        top_index = selected ? selected - 1 : selected;
    } else if (top_index + page_size <= selected + 1) {
        top_index = selected + 1 >= page_size ? (selected + 1 == sz ? selected + 1 - page_size : selected + 2 - page_size) : 0;
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

        auto &item = items[selected];
        if (item.type == menu_input) {
            std::string device_name, name;
            input->get_input_name(id, device_name, name);

            item.selected = id;
            item.str = name;
            if (item.callback) {
                item.callback(item);
            }
        }

        in_input_mode = false;
        input->clear_last_input();
        input->set_input_mode(drivers::input_base::mode_menu);
        return true;
    }
    auto states = input->input_state(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);
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
            } while(input->input_state(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK) != 0);
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
