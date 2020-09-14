#pragma once

#include "ttf_font_base.h"

namespace drivers {

class sdl2_ttf_ogl: public ttf_font_base {
public:
    ~sdl2_ttf_ogl() override;

    void render(int x, int y, const char *text, int width, int height, bool shadow = false);
    void set_draw_color(uint8_t r, uint8_t g, uint8_t b);

protected:
    const font_data *make_cache(uint16_t) override;

private:
    // std::vector<SDL_Texture*> textures, shadows;
    uint32_t color = 0xFFFFFFu;
};

}
