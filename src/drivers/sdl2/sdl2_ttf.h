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
    const font_data *make_cache(uint16_t) override;

private:
    SDL_Renderer *renderer;
    std::vector<SDL_Texture*> textures, shadows;
    uint32_t color = 0xFFFFFFu;
};

}
