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
        std::string name;
    };
public:
    sdl2_input();
    ~sdl2_input() override;

    void input_poll() override;

private:
    std::array<int, 16> keymap = {};
    std::vector<sdl2_game_pad> gamepad = {};
};

}
