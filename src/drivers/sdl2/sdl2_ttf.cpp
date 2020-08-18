#include "sdl2_ttf.h"

#include <SDL.h>

namespace drivers {

sdl2_ttf::~sdl2_ttf() {
    for (auto *t: textures) {
        SDL_DestroyTexture(t);
    }
    textures.clear();
}

const ttf_font::font_data *sdl2_ttf::make_cache(uint16_t ch) {
    const auto *fd = ttf_font::make_cache(ch);
    if (fd == nullptr) return fd;
    if (fd->rpidx >= textures.size()) {
        textures.resize(fd->rpidx + 1);
    }
    auto *&tex = textures[fd->rpidx];
    if (tex == nullptr) {
        auto w = get_rect_pack_width();
        tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, w);
    }
    void *data;
    int pitch;
    if (SDL_LockTexture(tex, nullptr, &data, &pitch) == 0) {
        auto *src_ptr = get_rect_pack_data(fd->rpidx, fd->rpx, fd->rpy);
        auto spacing = get_rect_pack_width() - fd->w;
        auto *pixels = (uint8_t*)data + fd->w * 4 + fd->h * pitch;
        auto pix_spacing = pitch - 4 * fd->w * 4;
        for (uint32_t j = 0; j < fd->h; ++j) {
            for (uint32_t i = 0; i < fd->w; ++i) {
                *(uint32_t*)pixels = 0xFFFFFFu | (((uint32_t)*src_ptr) << 24);
                pixels += 4;
                ++src_ptr;
            }
            src_ptr += spacing;
            pixels += pix_spacing;
        }
        SDL_UnlockTexture(tex);
    }
    return fd;
}

void sdl2_ttf::render(int x, int y, const char *text, int width, bool shadow) {

}

}
