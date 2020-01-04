#pragma once

#include <cstdint>

namespace drivers {

uint64_t get_ticks_usec();
void util_mkdir(const char *path);

}
