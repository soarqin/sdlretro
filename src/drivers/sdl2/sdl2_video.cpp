#include "sdl2_video.h"

#include "sdl2_ttf.h"

#include "cfg.h"

#include <spdlog/spdlog.h>

#include <SDL.h>

namespace drivers {

sdl2_video::sdl2_video() {
    std::tie(curr_width, curr_height) = g_cfg.get_resolution();
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    window = SDL_CreateWindow("SDLRetro", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              curr_width, curr_height, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        /* fallback to software renderer */
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    SDL_RendererInfo info = {};
    SDL_GetRendererInfo(renderer, &info);
    spdlog::info("SDL2 Driver: {}", info.name);

    ttf = std::make_shared<sdl2_ttf>(renderer);
    ttf->init(16, 0);
    ttf->add("font.ttf", 0);
}

sdl2_video::~sdl2_video() {
    if (texture) SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

bool sdl2_video::resolution_changed(unsigned width, unsigned height, unsigned pixel_format) {
    game_pixel_format = pixel_format;
    return true;
}

void sdl2_video::render(const void *data, unsigned width, unsigned height, size_t pitch) {
    if (!data) {
        drawn = false;
        return;
    }
    if (skip_frame) {
        drawn = false;
        skip_frame = false;
        return;
    }
    drawn = true;

    uint32_t pitch_in_pixel;
    switch(game_pixel_format) {
    case 0:
        pitch_in_pixel = pitch >> 1;
        break;
    case 1:
        pitch_in_pixel = pitch >> 2;
        break;
    default:
        pitch_in_pixel = pitch >> 1;
        break;
    }
    if (height != game_height || pitch_in_pixel != game_pixel_format) {
        if (texture) SDL_DestroyTexture(texture);
        game_height = height;
        game_pitch = pitch_in_pixel;
        float ratio;
        if (aspect_ratio <= 0.f) {
            ratio = (float)width / (float)height;
        } else {
            ratio = aspect_ratio;
        }
        auto expected_width = (int)std::lround(ratio * (float)curr_height);
        if (expected_width > curr_width) {
            auto expected_height = (int)std::lround((float)curr_width / ratio);
            display_rect = {0, ((int)curr_height - expected_height) / 2, (int)curr_width, expected_height};
        } else {
            display_rect = {((int)curr_width - expected_width) / 2, 0, expected_width, (int)curr_height};
        }
        switch (game_pixel_format) {
        case 0:
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_STREAMING, game_pitch, game_height);
            break;
        case 1:
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, game_pitch, game_height);
            break;
        default:
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, game_pitch, game_height);
            break;
        }
    }
    void *pixels;
    int lock_pitch;
    SDL_LockTexture(texture, nullptr, &pixels, &lock_pitch);
    memcpy(pixels, data, pitch * height);
    SDL_Rect rc {0, 0, (int)width, (int)height};
    SDL_Rect target_rc {display_rect[0], display_rect[1], display_rect[2], display_rect[3]};
    SDL_UnlockTexture(texture);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_RenderCopy(renderer, texture, &rc, &target_rc);
    SDL_RenderPresent(renderer);
}

void *sdl2_video::get_framebuffer(unsigned int *width, unsigned int *height, size_t *pitch, int *format) {
    return video_base::get_framebuffer(width, height, pitch, format);
}

void sdl2_video::clear() {
    SDL_RenderClear(renderer);
}

void sdl2_video::flip() {
    SDL_RenderPresent(renderer);
}

void sdl2_video::draw_text(int x, int y, const char *text, int width, bool shadow) {

}

uint32_t sdl2_video::get_text_width(const char *text) const {
    return 0;
}

}
