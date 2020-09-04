#include "sdl2_input.h"

#include <libretro.h>

#include <SDL.h>

namespace drivers {

sdl2_input::sdl2_input() {
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
    if (!SDL_WasInit(SDL_INIT_GAMECONTROLLER))
        SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    auto sz = SDL_NumJoysticks();
    gamepad.clear();
    for (int i = 0; i < sz; ++i) {
        if (!SDL_IsGameController(i)) continue;
        auto handle = SDL_GameControllerOpen(i);
        sdl2_game_pad pad;
        if (handle) {
            pad.handle = handle;
            pad.device_id = SDL_JoystickGetDeviceInstanceID(i);
            pad.name = SDL_GameControllerName(handle);
        } else {
            pad.handle = nullptr;
            pad.device_id = 0;
            pad.name.clear();
        }
        gamepad.emplace_back(pad);
    }
    for (size_t i = 0; i < keymap.size(); ++i) {
        map_key(keymap[i], 0, i);
        map_key(keymap[i], 0xFF, i);
    }
}

sdl2_input::~sdl2_input() {
    for (auto &pad: gamepad) {
        if (pad.handle) {
            SDL_GameControllerClose(pad.handle);
        }
    }
    gamepad.clear();
}

void sdl2_input::input_poll() {
    /*
    int numkeys;
    const uint8_t *keys = SDL_GetKeyboardState(&numkeys);
    uint16_t state = 0;
     */

    SDL_GameControllerUpdate();
    size_t pad_count = gamepad.size();
    for (size_t z = 0; z < ports.size(); ++z) {
        auto &port = ports[z];
        if (!port.enabled) continue;
        /*
        for (uint32_t i = 0; i < 16; ++i) {
            if (keys[keymap[i]]) state |= 1U << i;
        }
        port.states = static_cast<int16_t>(state);
         */

        if (z >= pad_count || !gamepad[z].handle) continue;
        port.analog_axis[0][0] = SDL_GameControllerGetAxis(gamepad[z].handle, SDL_CONTROLLER_AXIS_LEFTX);
        port.analog_axis[0][1] = SDL_GameControllerGetAxis(gamepad[z].handle, SDL_CONTROLLER_AXIS_LEFTY);
        port.analog_axis[1][0] = SDL_GameControllerGetAxis(gamepad[z].handle, SDL_CONTROLLER_AXIS_RIGHTX);
        port.analog_axis[1][1] = SDL_GameControllerGetAxis(gamepad[z].handle, SDL_CONTROLLER_AXIS_RIGHTY);
    }
}

void sdl2_input::port_connected(int index) {
    if (!SDL_IsGameController(index)) return;
    auto handle = SDL_GameControllerOpen(index);
    if (!handle) return;

    sdl2_game_pad *pad = nullptr;
    for (auto &p: gamepad) {
        if (p.handle != nullptr) continue;
        pad = &p;
        break;
    }
    if (pad == nullptr) {
        gamepad.resize(gamepad.size() + 1);
        pad = &gamepad.back();
    }

    pad->handle = handle;
    pad->device_id = SDL_JoystickGetDeviceInstanceID(index);
    pad->name = SDL_GameControllerName(handle);
}

void sdl2_input::port_disconnected(int device_id) {
    int found_port = -1;
    for (size_t i = 0; i < gamepad.size(); ++i) {
        auto &pad = gamepad[i];
        if (pad.device_id == device_id) {
            SDL_GameControllerClose(pad.handle);
            pad.handle = nullptr;
            pad.device_id = 0;
            pad.name.clear();
            found_port = i;
            break;
        }
    }
    if (found_port < 0) return;
    ++found_port;
    for (auto ite = game_mapping.begin(); ite != game_mapping.end();) {
        if ((ite->first >> 16) == found_port) {
            ite = game_mapping.erase(ite);
        } else {
            ++ite;
        }
    }
}

}
