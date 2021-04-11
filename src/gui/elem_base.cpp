#include "elem_base.h"

namespace gui {

void top_window::draw() {
    for (auto *c: children_ordered) {
        c->draw();
    }
}

void top_window::init() {
    gui_base::init();
}

void top_window::deinit() {
    gui_base::deinit();
}

}
