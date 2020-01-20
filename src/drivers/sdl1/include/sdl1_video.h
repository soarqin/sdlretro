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
    ~sdl1_video() override;
    bool resolution_changed(unsigned width, unsigned height, unsigned bpp) override;
    void render(const void *data, unsigned width, unsigned height, size_t pitch) override;
    void *get_framebuffer(unsigned *width, unsigned *height, size_t *pitch, int *format) override;
    bool frame_drawn() override { return drawn; }
    void clear();
    void flip();
    /* width: 0=fullscreen -1=fullscreen allow wrap
     *        others: negative = allow wrap */
    void draw_text(int x, int y, const char *text, int width = 0, bool shadow = false);
    uint32_t get_text_width(const char *text);

    inline unsigned get_width() { return curr_width; }
    inline unsigned get_height() { return curr_height; }
    inline sdl1_font *get_font() { return ttf.get(); }
    inline void set_force_scale(uint32_t s) { force_scale = s; }

private:
    void draw_text_pixel(int x, int y, const char *text, int width, bool shadow);

public:
    void enter_menu() override;
    void leave_menu() override;

private:
    SDL_Surface *screen = nullptr;
	void *screen_ptr = nullptr;
    std::shared_ptr<sdl1_font> ttf;
    uint32_t curr_width = 0, curr_height = 0, curr_bpp = 0;
    /* saved previous resolution for use with menu enter/leave */
    uint32_t saved_width = 0, saved_height = 0, saved_bpp = 0;

    /* override global scale cfg */
    uint32_t force_scale = 1;

    /* indicate wheather frame was drawn, for auto frameskip use */
    bool drawn = false;
};

}
