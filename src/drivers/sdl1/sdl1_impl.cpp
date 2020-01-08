#include "sdl1_impl.h"

#include "sdl1_video.h"
#include "sdl1_audio.h"
#include "sdl1_input.h"
#include "sdl1_font.h"
#include "throttle.h"

#include <core.h>

#include <SDL.h>

#include <cstdint>
#include <unistd.h>

namespace drivers {

sdl1_impl::sdl1_impl() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE) != 0) {
        return;
    }
    SDL_WM_SetCaption("SDLRetro", nullptr);
    video = std::make_shared<sdl1_video>();
    input = std::make_shared<sdl1_input>();
}

sdl1_impl::~sdl1_impl() {
    deinit();
}

bool sdl1_impl::process_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return true;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym ==
#ifdef GCW_ZERO
                    SDLK_HOME
#else
                    SDLK_ESCAPE
#endif
                    ) {
                    if (!in_game_menu)
                        in_game_menu = true;
                    else
                        return true;
                }
                break;
            default: break;
        }
    }
    return false;
}

bool sdl1_impl::init() {
    audio = std::make_shared<sdl1_audio>();
    return true;
}

void sdl1_impl::deinit() {
    SDL_Quit();
}

void sdl1_impl::unload() {
}

bool sdl1_impl::run_frame(std::function<void()> &in_game_menu_cb, bool check) {
    if (process_events()) return false;
    if (in_game_menu) {
        audio->pause(true);
        in_game_menu_cb();
        audio->pause(false);
        frame_throttle->reset(fps);
        in_game_menu = false;
    }
    if (check) {
        uint64_t usecs = 0;
        do {
            usleep(usecs);
            usecs = frame_throttle->check_wait();
        } while (usecs);
    } else
        frame_throttle->skip_check();
    return true;
}

}
