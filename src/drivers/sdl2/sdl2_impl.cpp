#include "sdl2_impl.h"

#include "sdl2_video.h"
#include "sdl2_audio.h"
#include "sdl2_input.h"
#include "throttle.h"

#include <SDL.h>

namespace drivers {

sdl2_impl::sdl2_impl() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return;
    }
    video = std::make_shared<sdl2_video>();
    input = std::make_shared<sdl2_input>();
    input->post_init();
}

sdl2_impl::~sdl2_impl() {
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
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP: {
            auto btn = sdl2_input::controller_button_map(event.cbutton.button);
            if (btn.first == 0) break;
            auto analog_index = btn.first >> 8;
            if (analog_index > 0) {
                input->on_axis_input(event.cbutton.which, ((analog_index - 1) << 1) + (btn.first & 0xFF), event.type == SDL_CONTROLLERBUTTONDOWN ? 0x7FFF : 0);
            } else {
                input->on_btn_input(event.cbutton.which, btn.first, event.type == SDL_CONTROLLERBUTTONDOWN);
            }
            break;
        }
        case SDL_CONTROLLERAXISMOTION: {
            auto btn = sdl2_input::controller_axis_map(event.caxis.axis, event.caxis.value);
            if (btn.first == 0) break;
            auto analog_index = btn.first >> 8;
            if (analog_index > 0) {
                input->on_axis_input(event.cbutton.which, ((analog_index - 1) << 1) + (btn.first & 0xFF), btn.second);
            } else {
                input->on_btn_input(event.cbutton.which, btn.first, btn.second != 0);
            }
            break;
        }
        case SDL_CONTROLLERDEVICEADDED:
            input->port_connected(event.cdevice.which);
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            input->port_disconnected(event.cdevice.which);
            break;
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
