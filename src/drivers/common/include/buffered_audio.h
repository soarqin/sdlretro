#pragma once

#include "circular_buffer.h"

#include <vector>
#include <cstdint>

namespace drivers {

class buffered_audio {
public:
    virtual ~buffered_audio() = default;

    bool start(bool mono, double sample_rate, double fps);
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
};

}
