#pragma once

#include "circular_buffer.h"

#include <vector>
#include <cstdint>

extern "C" typedef struct SRC_STATE_tag SRC_STATE;

namespace drivers {

class buffered_audio {
public:
    virtual ~buffered_audio() = default;

    bool start(bool mono, double sample_rate, unsigned sample_rate_out, double fps);
    void stop();

    void write_samples(const int16_t *data, size_t count);
    void read_samples(int16_t *data, size_t count);

protected:
    virtual bool open(unsigned) = 0;
    virtual void close() = 0;

public:
    virtual void pause(bool) = 0;

protected:
    bool mono_audio = false;
    unsigned output_sample_rate = 0;

private:
    circular_buffer<int16_t> buffer;
    std::vector<float> resampler_cache;

    /* for single divider */
    unsigned sample_multiplier = 1;
    int32_t sample_cache_sum[2] = {};
    int sample_cache_size = 0;

    /* for resampler */
    double sample_ratio = 1.;
    SRC_STATE *src_state = nullptr;
};

}
