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
    unsigned n = 1;
    while(n < 16) {
        if (output_sample_rate / n <= 48000) break;
        do {
            ++n;
        } while ((output_sample_rate % n) != 0 && n < 16);
    }
    sample_multiplier = n;
    output_sample_rate /= n;
    auto buffer_size = pullup(lround(output_sample_rate / fps));
    buffer.resize(buffer_size * 8);
    return open(buffer_size);
}

void buffered_audio::stop() {
    close();
    sample_multiplier = 1;
    sample_cache_size = 0;
}

void buffered_audio::write_samples(const int16_t *data, size_t count) {
    if (!output_sample_rate || !count) return;
    if (sample_multiplier == 1) {
        if (mono_audio) {
            size_t samples = count/2;
            int16_t monodata[128];
            do {
                size_t sz = samples > 128 ? 128 : samples;
                int16_t *ptr = monodata;
                for (size_t z = sz; z; z--) {
                    *ptr++ = (int16_t)(((int)data[0] + data[1])/2);
                    data += 2;
                }
                buffer.push(monodata, sz);
                samples -= sz;
            } while (samples);
        } else {
            buffer.push(data, count);
        }
    } else {
        count /= 2;
        int16_t sample_queue[128];
        size_t sample_queue_size = 0;
        do {
            auto to_write = sample_multiplier - sample_cache_size;
            if (count < to_write) to_write = count;
            for (int i = to_write; i; i--) {
                sample_cache_sum[0] += *data++;
                sample_cache_sum[1] += *data++;
            }
            sample_cache_size += to_write;
            if (sample_cache_size == sample_multiplier) {
                if (mono_audio) {
                    sample_queue[sample_queue_size++] = sample_cache_sum[0] + sample_cache_sum[1] / sample_multiplier / 2;
                } else {
                    sample_queue[sample_queue_size++] = (int16_t)(sample_cache_sum[0]/sample_multiplier);
                    sample_queue[sample_queue_size++] = (int16_t)(sample_cache_sum[1]/sample_multiplier);
                }
                sample_cache_sum[0] = sample_cache_sum[1] = 0;
                sample_cache_size = 0;
                if (sample_queue_size == 128) {
                    buffer.push(sample_queue, 128);
                    sample_queue_size = 0;
                }
            }
            count -= to_write;
        } while (count);
        if (sample_queue_size) {
            buffer.push(sample_queue, sample_queue_size);
        }
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
