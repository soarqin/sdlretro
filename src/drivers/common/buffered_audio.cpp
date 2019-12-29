#include "buffered_audio.h"

#include <memory.h>
#include <cmath>

namespace drivers {

buffered_audio::~buffered_audio() {
    deinit();
}

inline unsigned pullup(unsigned rate) {
    rate |= rate >> 1U;
    rate |= rate >> 2U;
    rate |= rate >> 4U;
    rate |= rate >> 8U;
    return (rate | (rate >> 16U)) + 1U;
}

bool buffered_audio::init(bool mono, double sample_rate, double fps) {
    output_sample_rate = lround(sample_rate);
    auto buffer_size = pullup(lround(output_sample_rate / fps));
    buffer.resize(buffer_size * 4);
    return open_audio(buffer_size);
}

void buffered_audio::deinit() {
    close_audio();
    buffer.clear();
}

void buffered_audio::write_samples(const int16_t *data, size_t count) {
    if (!count) return;
    buffer.push(data, count);
}

void buffered_audio::read_samples(int16_t *data, size_t count) {
    if (!count) return;
    size_t read_count = buffer.pop(data, count);
    if (read_count < count) {
        memset(data + read_count, 0, (count - read_count) * sizeof(int16_t));
    }
}

}
