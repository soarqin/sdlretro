#include "sdl2_impl.h"

#include "throttle.h"

namespace drivers {

sdl2_impl::sdl2_impl() {

}

sdl2_impl::~sdl2_impl() {

}

bool sdl2_impl::process_events() {
    return driver_base::process_events();
}

bool sdl2_impl::init() {
    return false;
}

void sdl2_impl::deinit() {

}

void sdl2_impl::unload() {

}

bool sdl2_impl::run_frame(std::function<void()> &in_game_menu_cb, bool check) {
    return false;
}

}
