#include "sdl1_impl.h"

#include "sdl1_video.h"
#include "sdl1_audio.h"
#include "sdl1_input.h"
#include "sdl1_ttf.h"
#include "throttle.h"

#include <core.h>

#include <SDL.h>

#include <cstdint>

namespace drivers {

sdl1_impl::sdl1_impl() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) != 0) {
        return;
    }
    SDL_WM_SetCaption("SDLRetro", nullptr);
    video = std::make_shared<sdl1_video>();
    input = std::make_shared<sdl1_input>();
}

sdl1_impl::~sdl1_impl() {
    video.reset();
    input.reset();
    audio.reset();
    SDL_Quit();
}

bool sdl1_impl::process_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            return true;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if (event.key.keysym.sym ==
#ifdef GCW_ZERO
                SDLK_HOME
#else
                SDLK_ESCAPE
#endif
                ) {
                if (event.type == SDL_KEYDOWN) {
                    if (!menu_button_pressed)
                        menu_button_pressed = true;
                    else
                        return true;
                }
            } else {
                input->on_key(event.key.keysym.sym, event.type == SDL_KEYDOWN);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            input->on_mouse(event.button.button, event.type == SDL_MOUSEBUTTONDOWN);
            break;
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
            input->on_joybtn(event.jbutton.which, event.jbutton.button, event.type == SDL_JOYBUTTONDOWN);
            break;
        default: break;
        }
    }
    return false;
}

bool sdl1_impl::init() {
    audio = std::make_shared<sdl1_audio>();
    static_cast<sdl1_video*>(video.get())->set_force_scale(0);
    return true;
}

void sdl1_impl::deinit() {
    static_cast<sdl1_video*>(video.get())->set_force_scale(1);
    audio.reset();
}

void sdl1_impl::unload() {
}

}
