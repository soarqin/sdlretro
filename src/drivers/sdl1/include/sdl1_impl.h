#pragma once

#include "driver_base.h"

namespace drivers {

class sdl1_impl: public driver_base {
public:
    using driver_base::driver_base;

    ~sdl1_impl() override;

protected:
    bool init() override;
    void deinit() final;
    bool run_frame() override;
};

}
