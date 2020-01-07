#pragma once

#include "video_base.h"

#include <memory>

extern "C" {
typedef struct SDL_Surface SDL_Surface;
}

namespace drivers {

class sdl1_font;

class sdl1_video: public video_base {
public:
    sdl1_video();
    bool resolution_changed(unsigned width, unsigned height, unsigned bpp) override;
    void render(const void *data, unsigned width, unsigned height, size_t pitch) override;
    void *get_framebuffer(unsigned *width, unsigned *height, size_t *pitch, int *format) override;
    bool frame_drawn() override { return drawn; }
    void draw_text(int x, int y, const char *text, bool allow_wrap, bool shadow);

    inline unsigned get_width() { return curr_width; }
    inline unsigned get_height() { return curr_height; }
    inline sdl1_font *get_font() { return ttf.get(); }

private:
    void draw_text_pixel(int x, int y, const char *text, bool allow_wrap, bool shadow);
    void clear();

public:
    void enter_menu() override;
    void leave_menu() override;

private:
    SDL_Surface *screen = nullptr;
    std::shared_ptr<sdl1_font> ttf;
    unsigned curr_width = 0, curr_height = 0, curr_bpp = 0;
    /* saved previous resolution for use with menu enter/leave */
    unsigned saved_width = 0, saved_height = 0, saved_bpp = 0;

    bool drawn = false;
};

}
