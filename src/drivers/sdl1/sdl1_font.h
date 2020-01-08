#pragma once

#include <ttf_font.h>

extern "C" {
typedef struct SDL_Surface SDL_Surface;
typedef struct SDL_Rect SDL_Rect;
}

namespace drivers {

class sdl1_font: public ttf_font {
public:
    void calc_depth_color(SDL_Surface *surface);

    void render(SDL_Surface *surface, int x, int y, const char *text, int width = 0, bool shadow = false);

private:
    uint32_t depth_color[256];
    uint32_t surface_bpp = 0;
};

}
