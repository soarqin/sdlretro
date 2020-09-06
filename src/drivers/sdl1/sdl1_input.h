#pragma once

#include "input_base.h"

#include <array>

namespace drivers {

class sdl1_input: public input_base {
public:
    sdl1_input();

    void input_poll() override;

    void port_connected(int index) override;

    void port_disconnected(int device_id) override;

    void get_input_name(uint64_t input, std::string &device_name, std::string &name) const override;
    uint64_t get_input_from_name(const std::string &device_name, const std::string &name) const override;

private:
    std::array<int, 16> keymap = {};
    std::array<void*, 2> joystick = {};
};

}
