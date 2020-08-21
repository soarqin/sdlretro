#pragma once

#include "audio_base.h"

namespace drivers {

class sdl2_audio: public audio_base {
public:
    using audio_base::audio_base;

    void reset() override;

protected:
    bool open(unsigned buffer_size) override;
    void close() override;
    void on_input(const int16_t *samples, size_t count) override;

public:
    void pause(bool b) override;

private:
    uint64_t next_check = 0;
    uint32_t device_id = 0;
    uint32_t max_queued_samples = 0;
};

}
