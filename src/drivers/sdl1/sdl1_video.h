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
    void window_resized(int width, int height, bool fullscreen) {}
    bool game_resolution_changed(int width, int height, int max_width, int max_height, unsigned pixel_format) override;
    void render(const void *data, int width, int height, size_t pitch) override;
    void *get_framebuffer(unsigned *width, unsigned *height, size_t *pitch, int *format) override;
    bool frame_drawn() override { return drawn; }
    void get_resolution(int &width, int &height) override { width = curr_width; height = curr_height; }
    int get_font_size() const override;
    void set_draw_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override;
    void draw_rectangle(int x, int y, int w, int h) override;
    void fill_rectangle(int x, int y, int w, int h) override;
    void draw_text(int x, int y, const char *text, int width, bool shadow) override;
    void get_text_width_and_height(const char *text, int &w, int &t, int &b) const override;
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
    int curr_width = 0, curr_height = 0;
    uint32_t curr_pixel_format = 0;
    /* saved previous resolution for use with menu enter/leave */
    int saved_width = 0, saved_height = 0;
    uint32_t saved_pixel_format = 0;

    /* override global scale cfg */
    int force_scale = 1;

    uint8_t draw_color[4] = {};

    /* indicate wheather frame was drawn, for auto frameskip use */
    bool drawn = false;
};

}
