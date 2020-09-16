#pragma once

#include "video_base.h"

#include <memory>

extern "C" {
typedef struct SDL_Window SDL_Window;
typedef void *SDL_GLContext;
}

namespace drivers {

class sdl2_ttf_ogl;

class sdl2_video_ogl: public video_base {
public:
    sdl2_video_ogl();
    ~sdl2_video_ogl() override;
    bool resolution_changed(unsigned width, unsigned height, unsigned pixel_format) override;
    void render(const void *data, unsigned width, unsigned height, size_t pitch) override;
    void *get_framebuffer(unsigned *width, unsigned *height, size_t *pitch, int *format) override;
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
    void get_text_width_and_height(const char *text, uint32_t &w, int &t, int &b) const override;

    void predraw_menu() override;
    void config_changed() override;

private:
    void do_render();
    void init_opengl();
    void uninit_opengl();

private:
    SDL_Window *window = nullptr;
    SDL_GLContext context = nullptr;

    uint32_t shader_direct_draw = 0, shader_texture = 0, shader_font = 0;
    uint32_t vao_draw = 0, vbo_draw = 0;
    uint32_t vao_texture = 0, vbo_texture = 0, ebo_texture = 0;
    uint32_t vao_font = 0, vbo_font = 0, ebo_font = 0;
    uint32_t texture_game = 0;
    uint32_t uniform_font_color = 0;
    float draw_color[4] = {1.f, 1.f, 1.f, 1.f};

    uint32_t curr_width = 0, curr_height = 0;
    uint32_t game_pitch = 0, game_width = 0, game_height = 0, game_pixel_format = 0;
    std::array<int, 4> display_rect = {};

    /* ttf[0] is regular font
     * fft[1] is bold font
     * */
    std::shared_ptr<sdl2_ttf_ogl> ttf[2];

    /* indicate wheather frame was drawn, for auto frameskip use */
    bool drawn = false;
};

}
