#pragma once

#include "buffered_audio.h"

namespace drivers {

class sdl2_audio: public buffered_audio {
public:
    using buffered_audio::buffered_audio;

protected:
    bool open(unsigned buffer_size) override;
    void close() override;

private:
    static void audio_callback(void *userdata, uint8_t *stream, int len);

public:
    void pause(bool b) override;

private:
    uint32_t device_id;
};

}
