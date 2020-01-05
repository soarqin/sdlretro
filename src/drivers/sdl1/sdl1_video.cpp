#include "sdl1_video.h"

#include "sdl1_font.h"

#include "cfg.h"

#include "bmfont.inl"

#include <SDL.h>

namespace drivers {

const int sdl_video_flags =
#ifdef GCW_ZERO
    SDL_FULLSCREEN |
#endif
    SDL_SWSURFACE |
#ifdef SDL_TRIPLEBUF
    SDL_TRIPLEBUF
#else
    SDL_DOUBLEBUF
#endif
    ;

sdl1_video::sdl1_video() {
    uint32_t w, h;
    std::tie(w, h) = g_cfg.get_resolution();
    curr_bpp = 16;
    screen = SDL_SetVideoMode(w, h, curr_bpp, sdl_video_flags);
}

bool sdl1_video::resolution_changed(unsigned width, unsigned height, unsigned bpp) {
    if (screen) {
        SDL_FreeSurface(screen);
        screen = nullptr;
    }
    curr_width = width;
    curr_height = height;
    curr_bpp = bpp;
    if (width != 0 && height != 0) {
        auto scale = g_cfg.get_scale();
        screen = SDL_SetVideoMode(width*scale, height*scale, bpp, sdl_video_flags);
    } else {
        uint32_t w, h;
        std::tie(w, h) = g_cfg.get_resolution();
        screen = SDL_SetVideoMode(w, h, bpp, sdl_video_flags);
    }
    return screen != nullptr;
}

void sdl1_video::render(const void *data, unsigned width, unsigned height, size_t pitch) {
    if (!data) return;

    if (curr_width != width || curr_height != height) {
        resolution_changed(width, height, curr_bpp);
    }
    bool lock = SDL_MUSTLOCK(screen);
    if (lock) SDL_LockSurface(screen);
    int h = static_cast<int>(height);
    auto scale = g_cfg.get_scale();
    auto bpp = curr_bpp;
    if (scale == 1) {
        auto *pixels = static_cast<uint8_t *>(screen->pixels);
        const auto *input = static_cast<const uint8_t *>(data);
        int output_pitch = screen->pitch;
        int line_bytes = width * (bpp >> 3);
        for (; h; h--) {
            memcpy(pixels, input, line_bytes);
            pixels += output_pitch;
            input += pitch;
        }
    } else {
    #define CODE_WITH_TYPE(TYPE) \
        auto *pixels = static_cast<TYPE*>(screen->pixels); \
        const auto *input = static_cast<const TYPE*>(data); \
        int output_pitch = screen->pitch / sizeof(TYPE); \
        auto s = scale; \
        auto subp = pitch / sizeof(TYPE) - width; \
        auto subd = output_pitch - s; \
        output_pitch *= scale - 1; \
        for (; h; h--) { \
            for (unsigned z = width; z; z--) { \
                auto pix = *input++; \
                auto *out = pixels; \
                for (int j = s; j; j--) { \
                    for (int i = s; i; i--) { \
                        *out++ = pix; \
                    } \
                    out += subd; \
                } \
                pixels += s; \
            } \
            pixels += output_pitch; \
            input += subp; \
        }
        if (bpp == 32) {
            CODE_WITH_TYPE(uint32_t)
        } else {
            CODE_WITH_TYPE(uint16_t)
        }
    }
    if (lock) SDL_UnlockSurface(screen);
    SDL_Flip(screen);
}

void *sdl1_video::get_framebuffer(unsigned *width, unsigned *height, size_t *pitch, int *format) {
    if (!screen) return nullptr;
    *width = screen->w;
    *height = screen->h;
    *pitch = screen->pitch;
    *format = 2;
    return screen->pixels;
}

}
