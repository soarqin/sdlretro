#include "sdl1_impl.h"

#include "sdl1_video.h"
#include "sdl1_audio.h"
#include "sdl1_input.h"
#include "throttle.h"

#include <core.h>

#include <SDL.h>

#include <cstdint>
#include <unistd.h>

namespace drivers {

sdl1_impl::~sdl1_impl() {
    deinit();
}

bool sdl1_impl::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE) != 0) {
        return false;
    }
    SDL_WM_SetCaption("SDLRetro", nullptr);
    video = std::make_unique<sdl1_video>();
    audio = std::make_unique<sdl1_audio>();
    input = std::make_unique<sdl1_input>();
    return true;
}

void sdl1_impl::deinit() {
    SDL_Quit();
}

void sdl1_impl::unload() {
}

bool sdl1_impl::run_frame() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return false;
            default: break;
        }
    }
    uint64_t usecs = 0;
    do {
        usleep(usecs);
        usecs = frame_throttle->check_wait();
    } while(usecs);
    return true;
}

}
