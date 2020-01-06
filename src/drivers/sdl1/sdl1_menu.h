#pragma once

#include "menu_base.h"

namespace drivers {

class sdl1_menu: public menu_base {
protected:
    void enter() override;
    void leave() override;
    void draw() override;
    bool poll_input() override;
    size_t page_count() override { return page_size; }

private:
    size_t page_size = 0;
};

}
