#pragma once

#include "audio_base.h"

#include "circular_buffer.h"

namespace drivers {

class sdl1_audio: public audio_base {
public:
    using audio_base::audio_base;

    void reset() override;

protected:
    bool open(unsigned buffer_size) override;
    void close() override;
    void on_input(const int16_t *samples, size_t count) override;

private:
    static void audio_callback(void *userdata, uint8_t *stream, int len);
    void read_samples(int16_t *data, size_t count);

public:
    void pause(bool b) override;

private:
    circular_buffer<int16_t> buffer;
};

}
