#pragma once

#include "../common/buffered_audio.h"

namespace drivers {

class sdl1_audio: public buffered_audio {
public:
    using buffered_audio::buffered_audio;

protected:
    bool open_audio(unsigned buffer_size) override;
    void close_audio() override;
};

}
