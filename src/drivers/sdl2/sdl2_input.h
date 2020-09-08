#pragma once

#include "input_base.h"

#include <vector>
#include <array>
#include <string>

extern "C" {
typedef struct _SDL_GameController SDL_GameController;
}

namespace drivers {

class sdl2_input: public input_base {
    struct sdl2_game_pad {
        SDL_GameController *handle;
        int device_id;
        std::string name;
    };
public:
    sdl2_input();
    ~sdl2_input() override;

    void post_init() override;

    void input_poll() override;
    void port_connected(int index) override;
    void port_disconnected(int device_id) override;
    void get_input_name(uint64_t input, std::string &device_name, std::string &name) const override;
    uint64_t get_input_from_name(const std::string &device_name, const std::string &name) const override;

private:
    std::vector<sdl2_game_pad> gamepad = {};
};

}
