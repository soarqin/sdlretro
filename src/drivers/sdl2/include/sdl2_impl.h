#pragma once

#include "driver_base.h"

namespace drivers {

class sdl2_impl: public driver_base {
public:
    sdl2_impl();

    ~sdl2_impl() override;

    bool process_events() final;

protected:
    bool init() final;
    void deinit() final;
    void unload() final;
};

}
