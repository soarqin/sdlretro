#pragma once

#include "input_base.h"

#include <array>

namespace drivers {

class sdl1_input: public input_base {
public:
    sdl1_input();

    void input_poll() override;

private:
    std::array<int, 16> keymap = {};
    void *joystick[2] = {nullptr, nullptr};
};

}
