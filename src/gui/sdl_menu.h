#pragma once

#include "menu_base.h"

namespace gui {

class sdl_menu: public menu_base {
public:
    using menu_base::menu_base;

protected:
    void enter() override;
    void leave() override;
    void draw() override;
    size_t page_count() override { return page_size; }

private:
    size_t page_size = 0;
    int line_height = 0;
    int title_x = 0;
    int top_most = 0, bot_most = 0;
    int key_x = 0;
    int value_x = 0;
    int value_width = 0;
    bool need_slider = false;
};

}
