#include "sdl2_input.h"

#include <libretro.h>

#include <SDL.h>

namespace drivers {

sdl2_input::sdl2_input() {
    if (!SDL_WasInit(SDL_INIT_GAMECONTROLLER))
        SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    SDL_GameControllerEventState(SDL_ENABLE);
}

sdl2_input::~sdl2_input() {
}

void sdl2_input::post_init() {
    input_base::post_init();

    std::array<uint16_t, 16>
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

    for (size_t i = 0; i < keymap.size(); ++i) {
        set_km_mapping(keymap[i], i);
    }
}

const char *INPUT_NAME_KEYBOARD = "keyboard";
const char *INPUT_NAME_MOUSE = "mouse";
const char *INPUT_NAME_MOUSE_LEFT = "LMButton";
const char *INPUT_NAME_MOUSE_RIGHT = "RMButton";
const char *INPUT_NAME_MOUSE_MIDDLE = "MMButton";

void sdl2_input::get_input_name(uint16_t id, std::string &device_name, std::string &name) const {
    if (id == 0) {
        device_name = "";
        name = "(none)";
        return;
    }
    if (id < 1024) {
        device_name = INPUT_NAME_KEYBOARD;
        name = SDL_GetScancodeName(static_cast<SDL_Scancode>(id));
    } else {
        device_name = INPUT_NAME_MOUSE;
        switch (id - 1024) {
        case SDL_BUTTON_LEFT:
            name = INPUT_NAME_MOUSE_LEFT;
            break;
        case SDL_BUTTON_RIGHT:
            name = INPUT_NAME_MOUSE_RIGHT;
            break;
        case SDL_BUTTON_MIDDLE:
            name = INPUT_NAME_MOUSE_MIDDLE;
            break;
        default:
            name = "Unknown";
            break;
        }
    }
}

uint16_t sdl2_input::get_input_from_name(const std::string &device_name, const std::string &name) const {
    if (device_name == INPUT_NAME_KEYBOARD) {
        return SDL_GetScancodeFromName(name.c_str());
    } else if (device_name == INPUT_NAME_MOUSE) {
        if (name == INPUT_NAME_MOUSE_LEFT) {
            return 1024 + SDL_BUTTON_LEFT;
        } else if (name == INPUT_NAME_MOUSE_RIGHT) {
            return 1024 + SDL_BUTTON_RIGHT;
        } else if (name == INPUT_NAME_MOUSE_MIDDLE) {
            return 1024 + SDL_BUTTON_MIDDLE;
        } else {
            return 1024 + 10;
        }
    }
    return 0;
}

}
