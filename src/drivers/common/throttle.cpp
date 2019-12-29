#include "throttle.h"

#include "util.h"

#include <cmath>

namespace drivers {

void throttle::reset(double fps) {
    frame_time = lround(1000000. / fps);
    next_frame = get_ticks_usec();
}

uint64_t throttle::check_wait() {
    uint64_t now = get_ticks_usec();
    if (now < next_frame)
        return next_frame - now;
    next_frame += frame_time;
    return 0ULL;
}

}
