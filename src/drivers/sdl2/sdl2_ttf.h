#pragma once

#include "ttf_font_base.h"

extern "C" {
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
}

namespace drivers {

class sdl2_ttf: public ttf_font_base {
public:
    inline explicit sdl2_ttf(SDL_Renderer *r): renderer(r) {}
    ~sdl2_ttf() override;

    void render(int x, int y, const char *text, int width, int height, bool shadow = false);
    void set_draw_color(uint8_t r, uint8_t g, uint8_t b);

protected:
    uint8_t *prepare_texture(size_t index, uint16_t x, uint16_t y, uint16_t w, uint16_t h, int &pitch) override;
    void finish_texture(uint8_t *src_ptr, size_t index, uint16_t x, uint16_t y, uint16_t w, uint16_t h, int src_pitch) override;

private:
    SDL_Renderer *renderer;
    std::vector<SDL_Texture*> textures, shadows;
    uint32_t color = 0xFFFFFFu;
};

}
