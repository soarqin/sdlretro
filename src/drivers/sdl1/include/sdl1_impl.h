#pragma once

#include "driver_base.h"

namespace drivers {

class sdl1_impl: public driver_base {
public:
    sdl1_impl();

    ~sdl1_impl() override;

    bool process_events() final;

protected:
    bool init() final;
    void deinit() final;
    void unload() final;
    bool run_frame(std::function<void()> &in_game_menu_cb, bool check) final;

private:
    bool in_game_menu = false;
};

}
