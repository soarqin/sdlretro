#pragma once

#include <cstdlib>

namespace drivers {

class video_base {
public:
    virtual ~video_base() = default;

    virtual bool init() = 0;
    virtual void deinit() = 0;
    virtual bool resolution_changed(unsigned width, unsigned height, unsigned bpp) = 0;
    virtual void render(const void *data, unsigned width, unsigned height, size_t pitch) = 0;

    inline void set_scale(unsigned s) { if (s > 0 && s < 5) scale = s; }

protected:
    // TODO: load default scale from config
    unsigned scale = 2;
};

}
