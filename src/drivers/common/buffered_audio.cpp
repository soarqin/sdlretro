#include "buffered_audio.h"

#include <memory.h>
#include <cmath>
#include <cstdlib>

namespace drivers {

inline unsigned pullup(unsigned rate) {
    rate |= rate >> 1U;
    rate |= rate >> 2U;
    rate |= rate >> 4U;
    rate |= rate >> 8U;
    return (rate | (rate >> 16U)) + 1U;
}

bool buffered_audio::start(bool mono, double sample_rate, double fps) {
    mono_audio = mono;
    output_sample_rate = lround(sample_rate);
    auto buffer_size = pullup(lround(output_sample_rate / fps));
    buffer.resize(buffer_size * 4);
    return open(buffer_size);
}

void buffered_audio::stop() {
    close();
}

void buffered_audio::write_samples(const int16_t *data, size_t count) {
    if (!output_sample_rate || !count) return;
    if (mono_audio) {
        size_t samples = count / 2;
        int16_t *monodata = (int16_t*)alloca(128 * sizeof(int16_t));
        do {
            size_t sz = samples > 128 ? 128 : samples;
            int16_t *ptr = monodata;
            for (size_t z = sz; z; z--) {
                *ptr++ = (int16_t)(((int)data[0] + data[1])/2);
                data += 2;
            }
            buffer.push(monodata, sz);
            samples -= sz;
        } while(samples);
    } else {
        buffer.push(data, count);
    }
}

void buffered_audio::read_samples(int16_t *data, size_t count) {
    if (!count) return;
    size_t read_count = buffer.pop(data, count);
    if (read_count < count) {
        memset(data + read_count, 0, (count - read_count) * sizeof(int16_t));
    }
}

}
