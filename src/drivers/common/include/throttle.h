#pragma once

#include <cstdint>

namespace drivers {

class throttle {
public:
    void reset(double fps);
    uint64_t check_wait();

private:
    uint64_t next_frame = 0;
    uint64_t frame_time = 0;
};

}
