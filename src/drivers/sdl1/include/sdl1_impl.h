#pragma once

#include "driver_base.h"

namespace drivers {

class sdl1_audio;

class sdl1_impl: public driver_base {
public:
    using driver_base::driver_base;

protected:
    bool init() override;
    void deinit() override;
    bool run_frame() override;
    void geometry_updated() override;

public:
    void video_refresh(const void *data, unsigned width, unsigned height, size_t pitch) override;
    void input_poll() override;
};

}
