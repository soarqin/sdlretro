#include "sdl1_video.h"

#include "sdl1_font.h"

#include "cfg.h"

#include "util.h"

#include <SDL.h>
#include <unistd.h>

namespace drivers {

const int sdl_video_flags = SDL_SWSURFACE |
#ifdef SDL_TRIPLEBUF
    SDL_TRIPLEBUF
#else
    SDL_DOUBLEBUF
#endif
    ;

sdl1_video::sdl1_video() {
    SDL_ShowCursor(SDL_DISABLE);
    std::tie(curr_width, curr_height) = g_cfg.get_resolution();
    curr_bpp = 16;
    screen = SDL_SetVideoMode(curr_width, curr_height, curr_bpp, sdl_video_flags);
    SDL_LockSurface(screen);
    screen_ptr = screen->pixels;
    /* TODO: ttf font load
    ttf = std::make_shared<sdl1_font>();
    ttf->init(16, 0);
    ttf->add("", 0);
     */
}

sdl1_video::~sdl1_video() {
    SDL_UnlockSurface(screen);
}

bool sdl1_video::resolution_changed(unsigned width, unsigned height, unsigned bpp) {
    SDL_UnlockSurface(screen);
    usleep(10000);
    curr_bpp = bpp;
    if (width != 0 && height != 0) {
        curr_width = width;
        curr_height = height;
        auto scale = g_cfg.get_scale();
        screen = SDL_SetVideoMode(width*scale, height*scale, bpp, sdl_video_flags);
    } else {
        std::tie(curr_width, curr_height) = g_cfg.get_resolution();
        screen = SDL_SetVideoMode(curr_width, curr_height, bpp, sdl_video_flags);
    }
    SDL_LockSurface(screen);
    screen_ptr = screen->pixels;
    return true;
}

void sdl1_video::render(const void *data, unsigned width, unsigned height, size_t pitch) {
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

    if (curr_width != width || curr_height != height) {
        resolution_changed(width, height, curr_bpp);
    }
    int h = static_cast<int>(height);
    auto scale = g_cfg.get_scale();
    auto bpp = curr_bpp;
    if (scale == 1) {
        auto *pixels = static_cast<uint8_t *>(screen_ptr);
        const auto *input = static_cast<const uint8_t *>(data);
        int output_pitch = screen->pitch;
        if (output_pitch == pitch) {
            memcpy(pixels, input, h * pitch);
        } else {
            int line_bytes = width*(bpp >> 3);
            for (; h; h--) {
                memcpy(pixels, input, line_bytes);
                pixels += output_pitch;
                input += pitch;
            }
        }
    } else {
    #define CODE_WITH_TYPE(TYPE) \
        auto *pixels = static_cast<TYPE*>(screen_ptr); \
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
    #undef CODE_WITH_TYPE
    }
    if (message_frames) {
        draw_text(0, curr_height * scale - 20, message_text.c_str(), 0, true);
    }
    flip();
}

void *sdl1_video::get_framebuffer(unsigned *width, unsigned *height, size_t *pitch, int *format) {
    if (!screen) return nullptr;
    *width = screen->w;
    *height = screen->h;
    *pitch = screen->pitch;
    *format = 2;
    return screen_ptr;
}

void sdl1_video::clear() {
    memset(screen_ptr, 0, screen->pitch * screen->h);
}

void sdl1_video::flip() {
    SDL_UnlockSurface(screen);
    SDL_Flip(screen);
    SDL_LockSurface(screen);
    screen_ptr = screen->pixels;
}

void sdl1_video::draw_text(int x, int y, const char *text, int width, bool shadow) {
    if (ttf) {
        ttf->render(screen, x, y, text, width, shadow);
    } else {
        draw_text_pixel(x, y, text, width, shadow);
    }
}

#include "bmfont.inl"

inline const font_data_t &get_pixel_font_data(uint8_t c) {
#ifdef GCW_ZERO
    return font_small_data[c];
#else
    return font_big_data[c];
#endif
}

uint32_t sdl1_video::get_text_width(const char *text) {
    uint32_t w = 0;
    if (ttf) {
        while (*text!=0) {
            uint32_t ch = utf8_to_ucs4(text);
            if (ch==0 || ch > 0xFFFFu) continue;
            w += ttf->get_char_width(ch);
        }
    } else {
        while (*text) {
            uint8_t c = *text++;
            if (c > 0x7F) continue;
            const auto &fd = get_pixel_font_data(c);
            w += fd.sw;
        }
    }
    return w;
}

void sdl1_video::draw_text_pixel(int x, int y, const char *text, int width, bool shadow) {
    bool allow_wrap = false;
    int nwidth;
    int ox = x;
    if (width == 0) {
        nwidth = width = screen->w - x;
    } else if (width == -1) {
        nwidth = width = screen->w - x;
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
    auto swidth = screen->pitch / screen->format->BytesPerPixel;
    while (*text) {
        uint8_t c = *text++;
        if (c > 0x7F) continue;
        const auto &fd = get_pixel_font_data(c);
        if (fd.sw > nwidth) {
            if (!allow_wrap) break;
            x = ox;
            nwidth = width;
#ifdef GCW_ZERO
            y += 9;
#else
            y += 18;
#endif
        }
        nwidth -= fd.sw;
    #define CODE_WITH_TYPE(TYPE) \
        auto *ptr = (TYPE*)screen_ptr + x + fd.x + (y + fd.y) * swidth; \
        auto *fontdata = fd.data; \
        uint32_t wrapx = swidth - fd.w; \
        uint32_t step = (fd.w + 7) >> 3; \
        if (shadow) { \
            for (int h = fd.h; h; h--) { \
                uint8_t bitflag = 0x01; \
                uint32_t fdidx = 0; \
                for (int w = fd.w; w; w--) { \
                    if (fontdata[fdidx] & bitflag) { \
                        *ptr++ = (TYPE)-1; \
                        *(ptr + swidth) = 0; \
                    } else ++ptr; \
                    if (bitflag == 0x80) { \
                        fdidx++; \
                        bitflag = 1; \
                    } else { \
                        bitflag <<= 1; \
                    } \
                } \
                ptr += wrapx; \
                fontdata += step; \
            } \
        } else { \
            for (int h = fd.h; h; h--) { \
                uint8_t bitflag = 0x01; \
                uint32_t fdidx = 0; \
                for (int w = fd.w; w; w--) { \
                    if (fontdata[fdidx] & bitflag) \
                        *ptr++ = (TYPE)-1; \
                    else ++ptr; \
                    if (bitflag == 0x80) { \
                        fdidx++; \
                        bitflag = 1; \
                    } else { \
                        bitflag <<= 1; \
                    } \
                } \
                ptr += wrapx; \
                fontdata += step; \
            } \
        }
        if (curr_bpp == 32) {
            CODE_WITH_TYPE(uint32_t)
        } else {
            CODE_WITH_TYPE(uint16_t)
        }
    #undef CODE_WITH_TYPE
        x += fd.sw;
    }
}

void sdl1_video::enter_menu() {
    SDL_UnlockSurface(screen);
    usleep(10000);
    saved_width = curr_width;
    saved_height = curr_height;
    saved_bpp = curr_bpp;
    std::tie(curr_width, curr_height) = g_cfg.get_resolution();
    curr_bpp = 16;
    screen = SDL_SetVideoMode(curr_width, curr_height, curr_bpp, sdl_video_flags);
    SDL_LockSurface(screen);
    screen_ptr = screen->pixels;
}

void sdl1_video::leave_menu() {
    resolution_changed(saved_width, saved_height, saved_bpp);
}

}
