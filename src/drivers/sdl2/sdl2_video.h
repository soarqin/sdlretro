#pragma once

#include "video_base.h"

#include <memory>
#include <array>

extern "C" {
typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
}

namespace drivers {

class sdl2_ttf;

class sdl2_video: public video_base {
public:
    sdl2_video();
    ~sdl2_video() override;
    void window_resized(int width, int height, bool fullscreen) override;
    bool game_resolution_changed(int width, int height, uint32_t pixel_format) override;
    void render(const void *data, int width, int height, size_t pitch) override;
    void *get_framebuffer(uint32_t *width, uint32_t *height, size_t *pitch, int *format) override;
    bool frame_drawn() override { return drawn; }
    void get_resolution(int &width, int &height) override {
        width = curr_width; height = curr_height;
    }
    void clear() override;
    void flip() override;

    int get_font_size() const override;
    void set_draw_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override;
    void draw_rectangle(int x, int y, int w, int h) override;
    void fill_rectangle(int x, int y, int w, int h) override;
    /* width: 0=fullscreen -1=fullscreen allow wrap
     *        others: negative = allow wrap */
    void draw_text(int x, int y, const char *text, int width, bool shadow) override;
    void get_text_width_and_height(const char *text, int &w, int &t, int &b) const override;

    void predraw_menu() override;
    void config_changed() override;

private:
    void do_render();
    void recalc_draw_rect();

private:
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;

    int curr_width = 0, curr_height = 0;
    int game_pitch = 0, game_width = 0, game_height = 0;
    uint32_t game_pixel_format = 0;
    std::array<int, 4> display_rect = {};

    /* ttf[0] is regular font
     * fft[1] is bold font
     * */
    std::shared_ptr<sdl2_ttf> ttf[2];

    /* indicate wheather frame was drawn, for auto frameskip use */
    bool drawn = false;
};

}
