#pragma once

#include "ttf_font_base.h"

namespace drivers {

class sdl2_ttf_ogl: public ttf_font_base {
public:
    inline sdl2_ttf_ogl(uint32_t shader, uint32_t vao, uint32_t vbo, uint32_t uniform): shader_font(shader), vao_font(vao), vbo_font(vbo), uniform_font_color(uniform) {}
    ~sdl2_ttf_ogl() override;

    void render(int x, int y, const char *text, int width, int height, bool shadow = false);
    void set_draw_color(uint8_t r, uint8_t g, uint8_t b);

protected:
    const font_data *make_cache(uint16_t) override;

private:
    std::vector<uint32_t> textures;
    uint32_t shader_font, vao_font, vbo_font, uniform_font_color;
    float color[3] = {1.f, 1.f, 1.f};
};

}
