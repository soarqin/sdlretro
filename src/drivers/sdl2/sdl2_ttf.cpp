#include "sdl2_ttf.h"

#include "util.h"

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
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    }
    void *data;
    int pitch;
    if (SDL_LockTexture(tex, nullptr, &data, &pitch) == 0) {
        auto *src_ptr = get_rect_pack_data(fd->rpidx, fd->rpx, fd->rpy);
        auto spacing = get_rect_pack_width() - fd->w;
        auto *pixels = (uint8_t*)data + fd->rpx * 4 + fd->rpy * pitch;
        auto pix_spacing = pitch - fd->w * 4;
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

void sdl2_ttf::render(int x, int y, const char *text, int width, int height, bool shadow) {
    if (font_size > height)
        return;
    int ox = x;
    int nwidth = width;
    bool allow_wrap = false;
    if (width < 0) {
        allow_wrap = true;
        width = -width;
    }
    while (*text != 0) {
        uint32_t ch = utf8_to_ucs4(text);
        if (ch == 0 || ch > 0xFFFFu) continue;

        const font_data *fd;
        auto ite = font_cache.find(ch);
        if (ite == font_cache.end()) {
            fd = make_cache(ch);
            if (!fd) {
                continue;
            }
        } else {
            fd = &ite->second;
            if (fd->advW == 0) continue;
        }

        if (fd->w > nwidth) {
            if (!allow_wrap) break;
            x = ox;
            nwidth = width;
            y += font_size;
            height -= font_size;
            if (y > height)
                break;
        }

        SDL_Rect src_rc = {fd->rpx, fd->rpy, fd->w, fd->h};
        SDL_Rect dst_rc = {x + fd->ix0, y + fd->iy0, fd->w, fd->h};
        SDL_RenderCopy(renderer, textures[fd->rpidx], &src_rc, &dst_rc);
        x += fd->advW;
        nwidth -= fd->advW;
    }
}

}
