#include "sdl1_video.h"

#include "sdl1_font.h"

#include "bmfont.inl"

#include <SDL.h>

namespace drivers {

// TODO: load default resolution from config
enum :unsigned {
    DEFAULT_WIDTH = 640,
    DEFAULT_HEIGHT = 480,
};

sdl1_video::sdl1_video() {
    screen = SDL_SetVideoMode(DEFAULT_WIDTH, DEFAULT_HEIGHT, 16, SDL_SWSURFACE | SDL_DOUBLEBUF);
}

sdl1_video::~sdl1_video() {
    if (screen) {
        SDL_FreeSurface(screen);
        screen = nullptr;
    }
}

bool sdl1_video::resolution_changed(unsigned width, unsigned height, unsigned bpp) {
    if (screen) {
        SDL_FreeSurface(screen);
        screen = nullptr;
    }
    if (width != 0 && height != 0)
        screen = SDL_SetVideoMode(width * scale, height * scale, 16, SDL_SWSURFACE | SDL_DOUBLEBUF);
    else
        screen = SDL_SetVideoMode(DEFAULT_WIDTH, DEFAULT_HEIGHT, 16, SDL_SWSURFACE | SDL_DOUBLEBUF);
    return screen != nullptr;
}

void sdl1_video::render(const void *data, unsigned width, unsigned height, size_t pitch) {
    if (!data) return;

    bool lock = SDL_MUSTLOCK(screen);
    if (lock) SDL_LockSurface(screen);
    int h = static_cast<int>(height);
    if (scale == 1) {
        auto *pixels = static_cast<uint8_t*>(screen->pixels);
        const auto *input = static_cast<const uint8_t*>(data);
        int output_pitch = screen->pitch;
        for (; h; h--) {
            memcpy(pixels, input, width*2);
            pixels += output_pitch;
            input += pitch;
        }
    } else {
        auto *pixels = static_cast<uint16_t*>(screen->pixels);
        const auto *input = static_cast<const uint16_t*>(data);
        int output_pitch = screen->pitch / 2;
        auto s = scale;
        auto subp = pitch / 2 - width;
        auto subd = output_pitch - s;
        output_pitch *= scale - 1;
        for (; h; h--) {
            for (unsigned z = width; z; z--) {
                auto pix = *input++;
                auto *out = pixels;
                for (int j = s; j; j--) {
                    for (int i = s; i; i--) {
                        *out++ = pix;
                    }
                    out += subd;
                }
                pixels += s;
            }
            pixels += output_pitch;
            input += subp;
        }
    }
    if (lock) SDL_UnlockSurface(screen);
    SDL_Flip(screen);
}

}
