#include "sdl1_ttf.h"

#include "util.h"

#include <SDL.h>

namespace drivers {

sdl1_ttf::~sdl1_ttf() {
    for (auto &p: pixels) {
        delete[] p;
    }
    pixels.clear();
}

void sdl1_ttf::calc_depth_color(SDL_Surface *surface) {
    surface_bpp = surface->format->BitsPerPixel;
    for (uint16_t i = 0; i < 256; ++i) {
        uint8_t c;
#ifndef TTF_SMOOTH
        if (i < 128) c = i * 3 / 2;
        else c = i / 2 + 128;
#else
        c = i;
#endif
        depth_color[i] = SDL_MapRGB(surface->format, c, c, c);
    }
}

void sdl1_ttf::render(SDL_Surface *surface, int x, int y, const char *text, int width, bool shadow) {
    if (surface->format->BitsPerPixel != surface_bpp) {
        calc_depth_color(surface);
    }
    auto rpw = get_rect_pack_width();

    bool allow_wrap = false;
    int nwidth;
    int ox = x;
    if (width == 0) {
        width = nwidth = surface->w - x;
    } else if (width == -1) {
        nwidth = width = surface->w - x;
        allow_wrap = true;
    } else {
        if (width < 0) {
            allow_wrap = true;
            width = -width;
            nwidth = width;
        } else {
            nwidth = width;
        }
    }
    int stride = surface->pitch / surface->format->BytesPerPixel;

    while (*text != 0) {
        uint32_t ch = util::utf8_to_ucs4(text);
        if (ch == 0 || ch > 0xFFFFu) continue;

        /* Check if bitmap is already cached */
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
        int cwidth = std::max(fd->advW, static_cast<uint8_t>((ch < (1u << 12u)) ? mono_width : mono_width * 2));
        if (cwidth > nwidth) {
            if (!allow_wrap) break;
            x = ox;
            nwidth = width;
            y += font_size + 1;
            if (y + font_size > surface->h)
                break;
        }
        nwidth -= cwidth;
    #define CODE_WITH_TYPE(TYPE) \
        TYPE *outptr = static_cast<TYPE*>(surface->pixels) + stride * (y + fd->iy0) + x + fd->ix0; \
        const uint8_t *input = &pixels[fd->rpidx][fd->rpy * rpw + fd->rpx]; \
        int iw = rpw - fd->w; \
        int ow = stride - fd->w; \
        for (int j = fd->h; j; j--) { \
            for (int i = fd->w; i; i--) { \
                uint8_t c; \
                if ((c = *input++) >= 32) { \
                    *outptr++ = depth_color[c]; \
                    if (shadow) \
                        *(outptr + stride) = 0; \
                } else \
                    ++outptr; \
            } \
            outptr += ow; \
            input += iw; \
        }

        if (surface_bpp == 32) {
            CODE_WITH_TYPE(uint32_t)
        } else {
            CODE_WITH_TYPE(uint16_t)
        }
    #undef CODE_WITH_TYPE
        x += cwidth;
    }
}

uint8_t *sdl1_ttf::prepare_texture(size_t index, uint16_t x, uint16_t y, uint16_t w, uint16_t h, int &pitch) {
    pitch = get_rect_pack_width();
    if (index >= pixels.size()) {
        pixels.resize(index + 1);
    }
    auto *&p = pixels[index];
    if (p == nullptr) {
        p = new uint8_t[pitch * pitch];
    }
    return &p[y * pitch + x];
}

}
