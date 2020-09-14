#include "sdl2_impl.h"

#ifdef USE_OPENGL
#include "sdl2_video_ogl.h"
#else
#include "sdl2_video.h"
#endif
#include "sdl2_audio.h"
#include "sdl2_input.h"
#include "throttle.h"

#include <SDL.h>

namespace drivers {

sdl2_impl::sdl2_impl() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return;
    }
#ifdef USE_OPENGL
    video = std::make_shared<sdl2_video_ogl>();
#else
    video = std::make_shared<sdl2_video>();
#endif
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
                input->on_key(event.key.keysym.scancode, event.type == SDL_KEYDOWN);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            input->on_mouse(event.button.button, event.type == SDL_MOUSEBUTTONDOWN);
            break;
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            input->on_joybtn(event.cbutton.which, event.cbutton.button, event.type == SDL_CONTROLLERBUTTONDOWN);
            break;
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
