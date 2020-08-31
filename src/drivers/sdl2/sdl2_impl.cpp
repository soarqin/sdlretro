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
            if (event.key.keysym.scancode ==
#ifdef GCW_ZERO
                SDL_SCANCODE_HOME
#else
                SDL_SCANCODE_ESCAPE
#endif
                ) {
                if (!menu_button_pressed)
                    menu_button_pressed = true;
                else
                    return true;
            } else {
                input->on_key(event.key.keysym.scancode, true);
            }
            break;
        case SDL_KEYUP:
            if (event.key.keysym.scancode ==
#ifdef GCW_ZERO
                SDL_SCANCODE_HOME
#else
                SDL_SCANCODE_ESCAPE
#endif
                ) {
            } else {
                input->on_key(event.key.keysym.scancode, false);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            input->on_mouse(event.button.button, true);
            break;
        case SDL_MOUSEBUTTONUP:
            input->on_mouse(event.button.button, false);
        case SDL_CONTROLLERBUTTONDOWN:
            input->on_joybtn(event.cbutton.which, event.cbutton.button, true);
            break;
        case SDL_CONTROLLERBUTTONUP:
            input->on_joybtn(event.cbutton.which, event.cbutton.button, true);
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
