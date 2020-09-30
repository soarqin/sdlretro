#include "sdl2_impl.h"

#include "sdl2_video.h"
#include "sdl2_audio.h"
#include "sdl2_input.h"
#include "throttle.h"

#include <SDL.h>

namespace drivers {

sdl2_impl::sdl2_impl() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        return;
    }
    video = std::make_shared<sdl2_video>();
    input = std::make_shared<sdl2_input>();
    input->post_init();
    SDL_JoystickEventState(SDL_ENABLE);
}

sdl2_impl::~sdl2_impl() {
    for (auto &p: joysticks) {
        SDL_JoystickClose(p.second);
    }
    joysticks.clear();

    video.reset();
    input.reset();
    audio.reset();
    SDL_Quit();
}

bool sdl2_impl::process_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            return true;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if (event.key.keysym.scancode ==
#ifdef GCW_ZERO
                SDL_SCANCODE_HOME
#else
                SDL_SCANCODE_ESCAPE
#endif
                ) {
                if (event.type == SDL_KEYDOWN) {
                    if (!menu_button_pressed)
                        menu_button_pressed = true;
                    else
                        return true;
                }
            } else {
                input->on_km_input(event.key.keysym.scancode, event.type == SDL_KEYDOWN);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            input->on_km_input(event.button.button + 1024, event.type == SDL_MOUSEBUTTONDOWN);
            break;
        case SDL_JOYDEVICEADDED: {
            auto *joystick = SDL_JoystickOpen(event.jdevice.which);
            auto device_id = SDL_JoystickInstanceID(joystick);
            joysticks[device_id] = joystick;
            auto device_guid = SDL_JoystickGetDeviceGUID(event.jdevice.which);
            gamecontrollerdb::GUID guid;
            memcpy(guid.data(), device_guid.data, sizeof(device_guid.data));
            input->on_device_connected(device_id, guid);
            break;
        }
        case SDL_JOYDEVICEREMOVED: {
            auto ite = joysticks.find(event.jdevice.which);
            if (ite != joysticks.end()) {
                SDL_JoystickClose(ite->second);
                joysticks.erase(ite);
            }
            input->on_device_disconnected(event.jdevice.which);
            break;
        }
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP: {
            input->on_joybtn_input(event.jbutton.which, event.jbutton.button, event.type == SDL_JOYBUTTONDOWN);
            break;
        }
        case SDL_JOYHATMOTION: {
            input->on_joyhat_input(event.jbutton.which, event.jhat.hat, event.jhat.value);
            break;
        }
        case SDL_JOYAXISMOTION: {
            input->on_joyaxis_input(event.jaxis.which, event.jaxis.axis, event.jaxis.value);
            break;
        }
        default: break;
        }
    }
    return false;
}

bool sdl2_impl::init() {
    audio = std::make_shared<sdl2_audio>();
    return true;
}

void sdl2_impl::deinit() {
    audio.reset();
}

void sdl2_impl::unload() {

}

}
