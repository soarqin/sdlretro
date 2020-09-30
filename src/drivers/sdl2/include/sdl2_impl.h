#pragma once

#include "driver_base.h"

#include <map>

extern "C" {
struct _SDL_Joystick;
typedef struct _SDL_Joystick SDL_Joystick;
}

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

private:
    std::map<uint32_t, SDL_Joystick*> joysticks;
};

}
