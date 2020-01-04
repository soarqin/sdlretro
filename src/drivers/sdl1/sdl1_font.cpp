#include "sdl1_font.h"

#include <SDL.h>

namespace drivers {

void sdl1_font::init_font(int size, uint8_t mono_width, SDL_Surface *surface) {
    init(size, mono_width);
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

void sdl1_font::render(SDL_Surface *surface, int x, int y, const char *text, bool allowWrap, bool shadow) {
    int stride = surface->pitch / surface->format->BytesPerPixel;
    int surface_w = surface->w;

    while (*text != 0) {
        uint32_t ch = utf8_to_ucs4(text);
        if (ch == 0 || ch > 0xFFFFu) continue;

        /* Check if bitmap is already cached */
        const font_data *fd;
        auto ite = font_cache.find(ch);

        if (ite == font_cache.end()) {
            fd = make_cache(ch);
        } else {
            fd = &ite->second;
            if (fd->advW == 0) continue;
        }
        int cwidth = std::max(fd->advW, static_cast<uint8_t>((ch < (1u << 12u)) ? mono_width : mono_width * 2));
        if (x + cwidth > surface_w) {
            if (!allowWrap) break;
            x = 0;
            y += font_size;
            if (y + font_size > surface->h)
                break;
        }
        uint16_t *outptr = static_cast<uint16_t*>(surface->pixels) + stride * (y + font_size + fd->iy0) + x + fd->ix0;
        const uint8_t *input = get_rect_pack_data(fd->rpidx, fd->rpx, fd->rpy);
        int iw = get_rect_pack_width() - fd->w;
        int ow = stride - fd->w;
        for (int j = fd->h; j; j--) {
            for (int i = fd->w; i; i--) {
#define TTF_NOALPHA
#ifdef TTF_NOALPHA
                uint8_t c;
                if ((c = *input++) >= 32) {
                    *outptr++ = depth_color[c];
                    if (shadow)
                        *(outptr + stride) = 0;
                } else
                    ++outptr;
#else
                uint16_t c = *outptr;
                if (c == 0) { *outptr++ = depth_color[*input++]; continue; }
                uint8_t n = *input++;
#ifndef USE_BGR15
                *outptr++ = ((((c >> 11u) * (0xFFu - n) + 0x1Fu * n) / 0xFFu) << 11u)
                    | (((((c >> 5u) & 0x3Fu) * (0xFFu - n) + 0x3Fu * n) / 0xFFu) << 5u)
                    | (((c & 0x1Fu) * (0xFFu - n) + 0x1Fu * n) / 0xFFu);
#else
	            *outptr++ = ((((c >> 10u) * (0xFFu - n) + 0x1Fu * n) / 0xFFu) << 10u)
	                        | (((((c >> 5u) & 0x1Fu) * (0xFFu - n) + 0x1Fu * n) / 0xFFu) << 5u)
	                        | (((c & 0x1Fu) * (0xFFu - n) + 0x1Fu * n) / 0xFFu);
#endif
#endif
            }
            outptr += ow;
            input += iw;
        }
        x += cwidth;
    }
}

}
