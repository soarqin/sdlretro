#include "menu_base.h"

#include "driver_base.h"
#include "video_base.h"

#include <unistd.h>

namespace drivers {

bool menu_base::enter_menu_loop() {
    if (topmenu) {
        driver->get_video()->enter_menu();
    }

    enter();
    running = true;
    while (running) {
        if (poll_input()) {
            draw();
        }
        usleep(50000);
    }
    leave();

    if (topmenu) {
        driver->get_video()->leave_menu();
    }
    return false;
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

}
