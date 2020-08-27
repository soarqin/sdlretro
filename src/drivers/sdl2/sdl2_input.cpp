#include "sdl2_input.h"

#include <libretro.h>

#include <SDL.h>

namespace drivers {

sdl2_input::sdl2_input() {
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
        SDL_SCANCODE_K, // RETRO_DEVICE_ID_JOYPAD_B
        SDL_SCANCODE_J, // RETRO_DEVICE_ID_JOYPAD_Y
        SDL_SCANCODE_C, // RETRO_DEVICE_ID_JOYPAD_SELECT
        SDL_SCANCODE_V, // RETRO_DEVICE_ID_JOYPAD_START
        SDL_SCANCODE_W, // RETRO_DEVICE_ID_JOYPAD_UP
        SDL_SCANCODE_S, // RETRO_DEVICE_ID_JOYPAD_DOWN
        SDL_SCANCODE_A, // RETRO_DEVICE_ID_JOYPAD_LEFT
        SDL_SCANCODE_D, // RETRO_DEVICE_ID_JOYPAD_RIGHT
        SDL_SCANCODE_L, // RETRO_DEVICE_ID_JOYPAD_A
        SDL_SCANCODE_I, // RETRO_DEVICE_ID_JOYPAD_X
        SDL_SCANCODE_Q, // RETRO_DEVICE_ID_JOYPAD_L
        SDL_SCANCODE_E, // RETRO_DEVICE_ID_JOYPAD_R
        SDL_SCANCODE_1, // RETRO_DEVICE_ID_JOYPAD_L2
        SDL_SCANCODE_3, // RETRO_DEVICE_ID_JOYPAD_R2
        SDL_SCANCODE_Z, // RETRO_DEVICE_ID_JOYPAD_L3
        SDL_SCANCODE_X, // RETRO_DEVICE_ID_JOYPAD_R3
    };
#endif
    if (!SDL_WasInit(SDL_INIT_JOYSTICK))
        SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    for (int i = 0; i < 2; ++i)
        joystick[i] = SDL_JoystickOpen(i);
}

void sdl2_input::input_poll() {
    int numkeys;
    const uint8_t *keys = SDL_GetKeyboardState(&numkeys);
    uint16_t state = 0;

    SDL_JoystickUpdate();
    for (size_t z = 0; z < 2; ++z) {
        auto &port = ports[z];
        if (!port.enabled) continue;
        for (uint32_t i = 0; i < 16; ++i) {
            if (keys[keymap[i]]) state |= 1U << i;
        }
        port.states = static_cast<int16_t>(state);

        port.analog_axis[0][0] = SDL_JoystickGetAxis((SDL_Joystick*)joystick[z], 0);
        port.analog_axis[0][1] = SDL_JoystickGetAxis((SDL_Joystick*)joystick[z], 1);
        port.analog_axis[1][0] = SDL_JoystickGetAxis((SDL_Joystick*)joystick[z], 2);
        port.analog_axis[1][1] = SDL_JoystickGetAxis((SDL_Joystick*)joystick[z], 3);
    }
}

}
