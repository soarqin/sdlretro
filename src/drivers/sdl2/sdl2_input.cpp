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
    for (auto &pad: gamepad) {
        if (pad.handle) {
            SDL_GameControllerClose(pad.handle);
        }
    }
    gamepad.clear();
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

    bool do_default_mapping = game_to_km_mapping.empty();
    if (do_default_mapping) {
        for (size_t i = 0; i < keymap.size(); ++i) {
            set_km_mapping(keymap[i], i);
        }
    }
    assign_port(0, 0);
}

void sdl2_input::port_connected(int index) {
    if (!SDL_IsGameController(index)) return;
    auto handle = SDL_GameControllerOpen(index);
    if (!handle) return;

    uint8_t port = 0;
    sdl2_game_pad *pad = nullptr;
    for (auto &p: gamepad) {
        if (p.handle == nullptr) {
            pad = &p;
            break;
        }
        ++port;
    }
    if (pad == nullptr) {
        gamepad.resize(gamepad.size() + 1);
        pad = &gamepad.back();
    }

    pad->handle = handle;
    pad->device_id = SDL_JoystickGetDeviceInstanceID(index);
    pad->name = SDL_GameControllerName(handle);

    if (port < ports.size()) {
        assign_port(pad->device_id, port);
    }
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
    if (found_port == 0) {
        assign_port(0, 0);
    } else {
        unassign_port(found_port);
    }
}

const char *INPUT_NAME_KEYBOARD = "keyboard";
const char *INPUT_NAME_MOUSE = "mouse";
const char *INPUT_NAME_MOUSE_LEFT = "LMButton";
const char *INPUT_NAME_MOUSE_RIGHT = "RMButton";
const char *INPUT_NAME_MOUSE_MIDDLE = "MMButton";

void sdl2_input::get_input_name(uint64_t input, std::string &device_name, std::string &name) const {
    if (input == 0) {
        device_name = "";
        name = "(none)";
        return;
    }
    auto id = static_cast<uint16_t>(input & 0xFFFFULL);
    auto device = static_cast<uint32_t>(input >> 16);
    if (device == 0) {
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
    } else {
        auto *c = SDL_GameControllerFromInstanceID(device);
        if (c == nullptr) {
            device_name = "Unknown";
        } else {
            device_name = SDL_GameControllerName(c);
        }
        name = SDL_GameControllerGetStringForButton(static_cast<SDL_GameControllerButton>(id));
    }
}

uint64_t sdl2_input::get_input_from_name(const std::string &device_name, const std::string &name) const {
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
    } else if (!gamepad.empty()) {
        for (auto &gp: gamepad) {
            if (gp.name == device_name) {
                return (static_cast<uint64_t>(gp.device_id) << 16) | static_cast<uint64_t>(SDL_GameControllerGetButtonFromString(name.c_str()));
            }
        }
        return (static_cast<uint64_t>(gamepad[0].device_id) << 16) | static_cast<uint64_t>(SDL_GameControllerGetButtonFromString(name.c_str()));
    }
    return 0;
}

std::pair<uint16_t, int16_t> sdl2_input::controller_button_map(uint16_t id) {
    switch (id) {
    case SDL_CONTROLLER_BUTTON_A: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_B, 0);
    case SDL_CONTROLLER_BUTTON_B: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_A, 0);
    case SDL_CONTROLLER_BUTTON_X: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_Y, 0);
    case SDL_CONTROLLER_BUTTON_Y: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_X, 0);
    case SDL_CONTROLLER_BUTTON_BACK: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_SELECT, 0);
    case SDL_CONTROLLER_BUTTON_START: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_START, 0);
    case SDL_CONTROLLER_BUTTON_LEFTSTICK: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_L3, 0);
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_R3, 0);
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_L, 0);
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_R, 0);
    case SDL_CONTROLLER_BUTTON_DPAD_UP: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_UP, 0);
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_DOWN, 0);
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_LEFT, 0);
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_RIGHT, 0);
    default: return std::make_pair(0, 0);
    }
}

std::pair<uint16_t, int16_t> sdl2_input::controller_axis_map(uint16_t index, int16_t value) {
    switch (index) {
    case SDL_CONTROLLER_AXIS_LEFTX: return std::make_pair(RETRO_DEVICE_ID_ANALOG_LX, value);
    case SDL_CONTROLLER_AXIS_LEFTY: return std::make_pair(RETRO_DEVICE_ID_ANALOG_LY, value);
    case SDL_CONTROLLER_AXIS_RIGHTX: return std::make_pair(RETRO_DEVICE_ID_ANALOG_RX, value);
    case SDL_CONTROLLER_AXIS_RIGHTY: return std::make_pair(RETRO_DEVICE_ID_ANALOG_RY, value);
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_L2, value >= 0x4000);
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: return std::make_pair(RETRO_DEVICE_ID_JOYPAD_R2, value >= 0x4000);
    default: return std::make_pair(0, 0);
    }
}

}
