#pragma once

#include "input_base.h"

#include <array>

extern "C" {
typedef struct _SDL_Joystick SDL_Joystick;
}

namespace drivers {

class sdl1_input: public input_base {
public:
    sdl1_input();

    ~sdl1_input() override;

    void post_init() override;

    void input_poll() override;

    void port_connected(int index) override;

    void port_disconnected(int device_id) override;

    void get_input_name(uint64_t input, std::string &device_name, std::string &name) const override;
    uint64_t get_input_from_name(const std::string &device_name, const std::string &name) const override;

private:
    std::array<int, 16> keymap = {};
    std::array<SDL_Joystick*, 2> joystick = {};
};

}
