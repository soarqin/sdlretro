#include "sdl1_input.h"

#include <libretro.h>

#include <SDL.h>

namespace drivers {

sdl1_input::sdl1_input() {
    keymap = {
        SDLK_k, // RETRO_DEVICE_ID_JOYPAD_B
        SDLK_j, // RETRO_DEVICE_ID_JOYPAD_Y
        SDLK_c, // RETRO_DEVICE_ID_JOYPAD_SELECT
        SDLK_v, // RETRO_DEVICE_ID_JOYPAD_START
        SDLK_w, // RETRO_DEVICE_ID_JOYPAD_UP
        SDLK_s, // RETRO_DEVICE_ID_JOYPAD_DOWN
        SDLK_a, // RETRO_DEVICE_ID_JOYPAD_LEFT
        SDLK_d, // RETRO_DEVICE_ID_JOYPAD_RIGHT
        SDLK_l, // RETRO_DEVICE_ID_JOYPAD_A
        SDLK_i, // RETRO_DEVICE_ID_JOYPAD_X
        SDLK_q, // RETRO_DEVICE_ID_JOYPAD_L
        SDLK_e, // RETRO_DEVICE_ID_JOYPAD_R
        SDLK_1, // RETRO_DEVICE_ID_JOYPAD_L2
        SDLK_3, // RETRO_DEVICE_ID_JOYPAD_R2
        SDLK_z, // RETRO_DEVICE_ID_JOYPAD_L3
        SDLK_x, // RETRO_DEVICE_ID_JOYPAD_R3
    };
}

void sdl1_input::input_poll() {
    int numkeys;
    uint8_t *keys = SDL_GetKeyState(&numkeys);
    uint16_t state = 0;
    for (uint32_t i = 0; i < 16; ++i) {
        if (keys[keymap[i]]) state |= 1U << i;
    }
    pad_states = static_cast<int16_t>(state);
}

}
