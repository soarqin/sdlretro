#pragma once

#include <ttf_font_base.h>
#include <bitset>

extern "C" {
typedef struct SDL_Surface SDL_Surface;
typedef struct SDL_Rect SDL_Rect;
}

namespace drivers {

class sdl1_ttf: public ttf_font_base {
public:
    ~sdl1_ttf() override;
    void set_draw_color(uint8_t r, uint8_t g, uint8_t b) override;

    uint32_t calc_depth_color(SDL_Surface *surface, uint8_t depth);
    void render(SDL_Surface *surface, int x, int y, const char *text, int width = 0, bool shadow = false);

protected:
    uint8_t *prepare_texture(size_t index, uint16_t x, uint16_t y, uint16_t w, uint16_t h, int &pitch) override;

private:
    uint32_t depth_color[256];
    std::bitset<256> depth_color_gen;
    uint32_t surface_bpp = 0;
    std::vector<uint8_t*> pixels;
    uint8_t color[3] = {0xFF, 0xFF, 0xFF};
};

}
