#include "sdl1_input.h"

#include <libretro.h>

#include <SDL.h>

namespace drivers {

sdl1_input::sdl1_input() {
#ifdef GCW_ZERO
    keymap = {
        SDLK_LALT,   // RETRO_DEVICE_ID_JOYPAD_B
        SDLK_LSHIFT, // RETRO_DEVICE_ID_JOYPAD_Y
        SDLK_ESCAPE, // RETRO_DEVICE_ID_JOYPAD_SELECT
        SDLK_RETURN, // RETRO_DEVICE_ID_JOYPAD_START
        SDLK_UP,     // RETRO_DEVICE_ID_JOYPAD_UP
        SDLK_DOWN,   // RETRO_DEVICE_ID_JOYPAD_DOWN
        SDLK_LEFT,   // RETRO_DEVICE_ID_JOYPAD_LEFT
        SDLK_RIGHT,  // RETRO_DEVICE_ID_JOYPAD_RIGHT
        SDLK_LCTRL,  // RETRO_DEVICE_ID_JOYPAD_A
        SDLK_SPACE,  // RETRO_DEVICE_ID_JOYPAD_X
        SDLK_TAB,    // RETRO_DEVICE_ID_JOYPAD_L
        SDLK_BACKSPACE, // RETRO_DEVICE_ID_JOYPAD_R
        SDLK_PAGEUP, // RETRO_DEVICE_ID_JOYPAD_L2
        SDLK_PAGEDOWN, // RETRO_DEVICE_ID_JOYPAD_R2
        SDLK_KP_DIVIDE, // RETRO_DEVICE_ID_JOYPAD_L3
        SDLK_KP_PERIOD, // RETRO_DEVICE_ID_JOYPAD_R3
    };
#else
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
#endif
}

void sdl1_input::input_poll() {
    int numkeys;
    uint8_t *keys = SDL_GetKeyState(&numkeys);
    uint16_t state = 0;
    for (int idx = 0; idx < 2; ++idx) {
        if (!pad_enabled[idx]) continue;
        for (uint32_t i = 0; i < 16; ++i) {
            if (keys[keymap[i]]) state |= 1U << i;
        }
        pad_states[idx] = static_cast<int16_t>(state);
    }
}

}
