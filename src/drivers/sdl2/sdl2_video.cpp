#include "sdl2_video.h"

#include "sdl2_ttf.h"

#include "cfg.h"

#include "util.h"

#include <spdlog/spdlog.h>

#include <SDL.h>

namespace drivers {

sdl2_video::sdl2_video() {
    std::tie(curr_width, curr_height) = g_cfg.get_resolution();
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
    support_render_to_texture = (info.flags & SDL_RENDERER_TARGETTEXTURE) != 0;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, g_cfg.get_linear() ? "1" : "0");
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    ttf[0] = std::make_shared<sdl2_ttf>(renderer);
    ttf[0]->init(16, 0);
    ttf[0]->add(g_cfg.get_data_dir() + PATH_SEPARATOR_CHAR + "fonts" + PATH_SEPARATOR_CHAR + "regular.ttf", 0);
    ttf[1] = std::make_shared<sdl2_ttf>(renderer);
    ttf[1]->init(16, 0);
    ttf[1]->add(g_cfg.get_data_dir() + PATH_SEPARATOR_CHAR + "fonts" + PATH_SEPARATOR_CHAR + "bold.ttf", 0);
}

sdl2_video::~sdl2_video() {
    if (background) SDL_DestroyTexture(background);
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
    if (width != game_width || height != game_height || pitch_in_pixel != game_pitch) {
        if (texture) SDL_DestroyTexture(texture);
        game_width = width;
        game_height = height;
        game_pitch = pitch_in_pixel;
        float ratio;
        if (aspect_ratio <= 0.f) {
            ratio = (float)width / (float)height;
        } else {
            ratio = aspect_ratio;
        }
        float sratio = (float)curr_width / (float)curr_height;
        if (g_cfg.get_integer_scaling()) {
            int int_ratio;
            if (sratio < ratio) {
                int_ratio = (int)std::floor((double)curr_width / width);
            } else {
                int_ratio = (int)std::floor((double)curr_height / height);
            }
            int expected_width = int_ratio * (int)width;
            int expected_height = int_ratio * (int)height;
            display_rect = {((int)curr_width - expected_width) / 2, ((int)curr_height - expected_height) / 2, expected_width, expected_height};
        } else {
            if (sratio < ratio) {
                auto expected_height = (int)std::lround((float)curr_width / ratio);
                display_rect = {0, ((int)curr_height - expected_height) / 2, (int)curr_width, expected_height};
            } else {
                auto expected_width = (int)std::lround(ratio * (float)curr_height);
                display_rect = {((int)curr_width - expected_width) / 2, 0, expected_width, (int)curr_height};
            }
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
    if (SDL_LockTexture(texture, nullptr, &pixels, &lock_pitch) == 0) {
        memcpy(pixels, data, pitch * height);
        SDL_UnlockTexture(texture);
    }
    do_render();
    SDL_RenderPresent(renderer);
}

void *sdl2_video::get_framebuffer(unsigned int *width, unsigned int *height, size_t *pitch, int *format) {
    return video_base::get_framebuffer(width, height, pitch, format);
}

void sdl2_video::clear() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void sdl2_video::flip() {
    SDL_RenderPresent(renderer);
}

int sdl2_video::get_font_size() const {
    return ttf[0]->get_font_size();
}

void sdl2_video::set_draw_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void sdl2_video::draw_rectangle(int x, int y, int w, int h) {
    SDL_Rect rc{x, y, w, h};
    SDL_RenderDrawRect(renderer, &rc);
}

void sdl2_video::fill_rectangle(int x, int y, int w, int h) {
    SDL_Rect rc{x, y, w, h};
    SDL_RenderFillRect(renderer, &rc);
}

void sdl2_video::draw_text(int x, int y, const char *text, int width, bool shadow) {
    if (width == 0) width = (int)curr_width - x;
    else if (width < 0) width = x - (int)curr_width;
    ttf[0]->render(x, y, text, width, (int)curr_height + ttf[0]->get_font_size() - y, shadow);
}

void sdl2_video::get_text_width_and_height(const char *text, uint32_t &w, int &t, int &b) const {
    w = 0;
    t = 255;
    b = -255;
    while (*text != 0) {
        uint32_t ch = util::utf8_to_ucs4(text);
        if (ch == 0 || ch > 0xFFFFu) continue;
        uint8_t width;
        int8_t tt, tb;
        ttf[0]->get_char_width_and_height(ch, width, tt, tb);
        if (width) {
            w += width;
            if (tt < t) t = tt;
            if (tb > b) b = tb;
        }
    }
}

void sdl2_video::predraw_menu() {
    SDL_Rect rc{0, 0, (int)game_width, (int)game_height};
    SDL_Rect target_rc{display_rect[0], display_rect[1], display_rect[2], display_rect[3]};
    SDL_RenderCopy(renderer, texture, &rc, &target_rc);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xA0);
    SDL_RenderFillRect(renderer, nullptr);
}

void sdl2_video::config_changed() {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, g_cfg.get_linear() ? "1" : "0");
    game_width = game_height = game_pitch = 0;
}

void sdl2_video::do_render() {
    clear();

    SDL_Rect rc{0, 0, (int)game_width, (int)game_height};
    SDL_Rect target_rc{display_rect[0], display_rect[1], display_rect[2], display_rect[3]};
    SDL_RenderCopy(renderer, texture, &rc, &target_rc);

    if (messages.empty()) return;
    uint32_t lh = ttf[0]->get_font_size() + 2;
    uint32_t y = curr_height - 5 - (messages.size() - 1) * lh;
    for (auto &m: messages) {
        draw_text(5, y, m.first.c_str(), 0, true);
        y += lh;
    }
}

}
