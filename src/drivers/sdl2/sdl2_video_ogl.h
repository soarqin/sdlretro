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

    int get_renderer_type() override;
    bool init_hw_renderer(retro_hw_render_callback*) override;
    void inited_hw_renderer() override;
    void uninit_hw_renderer() override;
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
    void get_text_width_and_height(const char *text, uint32_t &w, int &t, int &b) const override;

    void predraw_menu() override;
    void config_changed() override;

    inline uintptr_t get_hw_fbo() const { return hw_renderer.fbo; }

private:
    void do_render();
    void init_opengl();
    void uninit_opengl();
    void gl_set_ortho();
    bool recalc_draw_rect(bool force_create_empty_texture = false);

    void gl_renderer_update_texture_rect(bool force_create_empty_texture = false);
    void gl_renderer_create_empty_texture() const;
    bool gl_renderer_resized(float wratio, float hratio) const;
    bool gl_renderer_gen_texture(const void *data, size_t pitch) const;

private:
    SDL_Window *window = nullptr;
    SDL_GLContext context = nullptr;

    struct {
        uint32_t shader_direct_draw = 0, shader_texture = 0, shader_font = 0;
        uint32_t vao_draw = 0, vbo_draw = 0;
        uint32_t vao_texture = 0, vbo_texture = 0;
        uint32_t vao_font = 0, vbo_font = 0;
        uint32_t texture_game = 0;
        uint32_t texture_w = 0, texture_h = 0;
        uint32_t uniform_font_color = 0;
        float draw_color[4] = {1.f, 1.f, 1.f, 1.f};
        bool bottom_left = false;
    } gl_renderer;

    int curr_width = 0, curr_height = 0;
    size_t bpp = 2;
    int game_width = 0, game_height = 0;
    uint32_t game_pixel_format = 0;

    /* ttf[0] is regular font
     * fft[1] is bold font
     * */
    std::shared_ptr<sdl2_ttf_ogl> ttf[2];

    /* indicate wheather frame was drawn, for auto frameskip use */
    bool drawn = false;

    retro_hw_render_callback *hwr_cb = nullptr;

    struct {
        uint32_t fbo = 0;
        uint32_t rb_ds = 0;
    } hw_renderer;
};

}
