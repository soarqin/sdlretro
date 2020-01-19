#include "buffered_audio.h"

#include "cfg.h"
#include "samplerate.h"

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

bool buffered_audio::start(bool mono, double sample_rate_in, unsigned sample_rate_out, double fps) {
    mono_audio = mono;
    if (sample_rate_out == 0) {
        output_sample_rate = lround(sample_rate_in);
        unsigned n = 1;
        while(n < 16) {
            if (output_sample_rate / n <= 48000) break;
            do {
                ++n;
            } while ((output_sample_rate % n) != 0 && n < 16);
        }
        sample_multiplier = n;
        output_sample_rate /= n;
    } else {
        src_state = src_new(g_cfg.get_resampler_quality() > 4 ? 0 : (int)(4 - g_cfg.get_resampler_quality()), 2, nullptr);
        output_sample_rate = sample_rate_out;
        sample_ratio = output_sample_rate / sample_rate_in;
    }
    auto buffer_size = pullup(lround(output_sample_rate / fps));
    buffer.resize(buffer_size * 8);
    resampler_cache.clear();
    return open(buffer_size);
}

void buffered_audio::stop() {
    close();
    sample_multiplier = 1;
    sample_cache_size = 0;

    src_delete(src_state);
    src_state = nullptr;
    sample_ratio = 1.;
    resampler_cache.reserve(65536);
}

enum :size_t {
    input_buffer_size = 2048,
    output_buffer_size = 2048,
};

void buffered_audio::write_samples(const int16_t *data, size_t count) {
    if (!output_sample_rate || !count) return;
    if (src_state) {
        if (resampler_cache.size() < 65536) {
            float outdata[input_buffer_size * 2];
            while (count) {
                size_t pcount = count > input_buffer_size * 2 ? input_buffer_size * 2 : count;
                src_short_to_float_array(data, outdata, pcount);
                if (pcount + resampler_cache.size() >= 65536) {
                    resampler_cache.insert(resampler_cache.end(), outdata, outdata + (65536 - resampler_cache.size()));
                    break;
                }
                resampler_cache.insert(resampler_cache.end(), outdata, outdata + pcount);
                count -= pcount;
                data += pcount;
            }
        }
        float proc_data[output_buffer_size * 2];
        int16_t proc_data_s[output_buffer_size * 2];
        float *data_ptr = &resampler_cache[0];
        size_t cache_size = resampler_cache.size();
        do {
            SRC_DATA src_data = {data_ptr, proc_data, (long)cache_size / 2, output_buffer_size, 0, 0, 0, sample_ratio};
            int res = src_process(src_state, &src_data);
            if (res != 0) {
                fprintf(stderr, "%s\n", src_strerror(res));
                return;
            }
            if (src_data.output_frames_gen) {
                if (mono_audio) {
                    auto *inptr = proc_data;
                    auto *outptr = proc_data;
                    for (auto z = src_data.output_frames_gen; z; z--) {
                        *outptr++ = (inptr[0] + inptr[1]) / 2;
                        inptr += 2;
                    }
                    src_float_to_short_array(proc_data, proc_data_s, src_data.output_frames_gen);
                    buffer.push(proc_data_s, src_data.output_frames_gen);
                } else {
                    src_float_to_short_array(proc_data, proc_data_s, src_data.output_frames_gen * 2);
                    buffer.push(proc_data_s, src_data.output_frames_gen * 2);
                }
            }
            if (!src_data.input_frames_used) break;
            data_ptr += src_data.input_frames_used * 2;
            cache_size -= src_data.input_frames_used * 2;
        } while (cache_size);
        resampler_cache.erase(resampler_cache.begin(), resampler_cache.begin() + (resampler_cache.size() - cache_size));
    } else {
        if (sample_multiplier == 1) {
            if (mono_audio) {
                size_t samples = count/2;
                int16_t monodata[output_buffer_size];
                do {
                    size_t sz = samples > output_buffer_size ? output_buffer_size : samples;
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
            int16_t sample_queue[output_buffer_size];
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
                    if (sample_queue_size == output_buffer_size) {
                        buffer.push(sample_queue, output_buffer_size);
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
}

void buffered_audio::read_samples(int16_t *data, size_t count) {
    if (!count) return;
    size_t read_count = buffer.pop(data, count);
    if (read_count < count) {
        memset(data + read_count, 0, (count - read_count) * sizeof(int16_t));
    }
}

}
