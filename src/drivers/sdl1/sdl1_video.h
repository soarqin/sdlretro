#pragma once

#include "video_base.h"

#include <memory>

extern "C" {
typedef struct SDL_Surface SDL_Surface;
}

namespace drivers {

class sdl1_ttf;

class sdl1_video: public video_base {
public:
    sdl1_video();
    ~sdl1_video() override;
    bool resolution_changed(unsigned width, unsigned height, unsigned pixel_format) override;
    void render(const void *data, unsigned width, unsigned height, size_t pitch) override;
    void *get_framebuffer(unsigned *width, unsigned *height, size_t *pitch, int *format) override;
    bool frame_drawn() override { return drawn; }
    void get_resolution(int &width, int &height) override { width = curr_width; height = curr_height; }
    int get_font_size() const override;
    void draw_rectangle(int x, int y, int w, int h) override;
    void draw_text(int x, int y, const char *text, int width, bool shadow) override;
    void get_text_width_and_height(const char *text, uint32_t &w, int &t, int &b) const override;
    void clear() override;
    void flip() override;

    inline void set_force_scale(uint32_t s) { force_scale = s; }

private:
    void draw_text_pixel(int x, int y, const char *text, int width, bool shadow);

public:
    void enter_menu() override;
    void leave_menu() override;

private:
    SDL_Surface *screen = nullptr;
	void *screen_ptr = nullptr;
    std::shared_ptr<sdl1_ttf> ttf[2];
    uint32_t curr_width = 0, curr_height = 0, curr_pixel_format = 0;
    /* saved previous resolution for use with menu enter/leave */
    uint32_t saved_width = 0, saved_height = 0, saved_pixel_format = 0;

    /* override global scale cfg */
    uint32_t force_scale = 1;

    /* indicate wheather frame was drawn, for auto frameskip use */
    bool drawn = false;
};

}
