#pragma once

#include "driver_base.h"

namespace drivers {

class sdl2_impl: public driver_base {
public:
    sdl2_impl();

    ~sdl2_impl() override;

    bool process_events() final;
    inline bool get_menu_button_pressed() const { return menu_button_pressed; }

protected:
    bool init() final;
    void deinit() final;
    void unload() final;
    bool run_frame(std::function<void()> &in_game_menu_cb, bool check) final;

private:
    bool menu_button_pressed = false;
};

}
