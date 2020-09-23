#pragma once

#include "ttf_font_base.h"

namespace drivers {

class sdl2_ttf: public ttf_font_base {
public:
    inline sdl2_ttf(uint32_t shader, uint32_t vao, uint32_t vbo, uint32_t uniform): program_font(shader), vao_font(vao), vbo_font(vbo), uniform_font_color(uniform) {}
    ~sdl2_ttf() override;

    void render(int x, int y, const char *text, int width, int height, bool shadow = false);
    void set_draw_color(uint8_t r, uint8_t g, uint8_t b);

protected:
    uint8_t *prepare_texture(size_t index, uint16_t x, uint16_t y, uint16_t w, uint16_t h, int &pitch) override;
    void finish_texture(uint8_t *data, size_t index, uint16_t x, uint16_t y, uint16_t w, uint16_t h, int pitch) override;

private:
    std::vector<uint32_t> textures;
    uint32_t program_font, vao_font, vbo_font, uniform_font_color;
    float color[3] = {1.f, 1.f, 1.f};
};

}
