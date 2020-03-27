#pragma once

#include <cstdint>

uint64_t get_ticks_usec();
int util_mkdir(const char *path);
uint32_t utf8_to_ucs4(const char *&text);
