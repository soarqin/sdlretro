#pragma once

#include "video_base.h"

extern "C" {
typedef struct SDL_Surface SDL_Surface;
}

namespace drivers {

class sdl1_video: public video_base {
public:
    sdl1_video();
    ~sdl1_video() override;
    bool resolution_changed(unsigned width, unsigned height, unsigned bpp) override;
    void render(const void *data, unsigned width, unsigned height, size_t pitch) override;

private:
    SDL_Surface *screen = nullptr;
};

}
