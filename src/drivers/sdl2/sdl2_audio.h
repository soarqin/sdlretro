#pragma once

#include "buffered_audio.h"

namespace drivers {

class sdl2_audio: public buffered_audio {
public:
    using buffered_audio::buffered_audio;

    void reset() override;

protected:
    bool open(unsigned buffer_size) override;
    void close() override;
    void on_input() override;

/*
private:
    static void audio_callback(void *userdata, uint8_t *stream, int len);
*/
public:
    void pause(bool b) override;

private:
    uint32_t device_id = 0;
    uint32_t max_queued_samples = 0;
};

}
