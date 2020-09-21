#pragma once

#include <vector>
#include <cstdint>

extern "C" typedef struct SRC_STATE_tag SRC_STATE;

namespace drivers {

class audio_base {
public:
    virtual ~audio_base() = default;

    bool start(bool mono, double sample_rate, unsigned sample_rate_out, double fps);
    void stop();
    virtual void reset();

    double get_sample_rate_input() const { return sample_rate_input; }

    void write_samples(const int16_t *data, size_t count);

protected:
    virtual bool open(unsigned) = 0;
    virtual void close() = 0;
    virtual void on_input(const int16_t *samples, size_t count) {}

public:
    virtual void pause(bool) = 0;

protected:
    bool mono_audio = false;
    unsigned output_sample_rate = 0;

private:
    std::vector<float> resampler_cache;
    double sample_rate_input = 0.;

    /* for single divider */
    unsigned sample_multiplier = 1;
    int32_t sample_cache_sum[2] = {};
    int sample_cache_size = 0;

    /* for resampler */
    double sample_ratio = 1.;
    SRC_STATE *src_state = nullptr;
};

}
