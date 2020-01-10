#pragma once

#include <cstdint>

namespace drivers {

class throttle {
public:
    void reset(double fps);
    int64_t check_wait();
    void skip_check();

private:
    uint64_t next_frame = 0;
    uint64_t frame_time = 0;
};

}
