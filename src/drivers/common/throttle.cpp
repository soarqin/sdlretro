#include "throttle.h"

#include "util.h"

#include <cmath>

namespace drivers {

void throttle::reset(double fps) {
    frame_time = lround(1000000. / fps);
    next_frame = get_ticks_usec();
}

int64_t throttle::check_wait() {
    uint64_t now = get_ticks_usec();
    auto result = static_cast<int64_t>(next_frame - now);
    if (result > 0)
        return result;
    next_frame += frame_time;
    return result;
}

void throttle::skip_check() {
    next_frame += frame_time;
}

}
