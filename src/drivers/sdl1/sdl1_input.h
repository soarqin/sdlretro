#pragma once

#include "input_base.h"

#include <array>

namespace drivers {

class sdl1_input: public input_base {
public:
    void init() override;
    void deinit() override;

    void input_poll() override;

private:
    std::array<int, 16> keymap = {};
};

}
