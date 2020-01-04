#include <util.h>

#include <time.h>

namespace drivers {

uint64_t get_ticks_usec() {
#ifndef CLOCK_MONOTONIC_COARSE
#define CLOCK_MONOTONIC_COARSE CLOCK_MONOTONIC
#endif
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    return ts.tv_sec*1000000ULL + ts.tv_nsec/1000ULL;
}

}
