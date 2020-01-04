#pragma once

#include <cstdlib>

namespace drivers {

class video_base {
public:
    virtual ~video_base() = default;

    virtual bool resolution_changed(unsigned width, unsigned height, unsigned bpp) = 0;
    virtual void render(const void *data, unsigned width, unsigned height, size_t pitch) = 0;
};

}
