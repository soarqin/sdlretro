#pragma once

#include "input_base.h"

#include <array>

namespace drivers {

class sdl2_input: public input_base {
public:
    sdl2_input();

    void input_poll() override;

private:
    std::array<int, 16> keymap = {};
    std::array<void*, 2> joystick = {};
};

}
