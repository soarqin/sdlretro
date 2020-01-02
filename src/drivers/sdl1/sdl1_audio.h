#pragma once

#include "buffered_audio.h"

namespace drivers {

class sdl1_audio: public buffered_audio {
public:
    using buffered_audio::buffered_audio;

protected:
    bool open(unsigned buffer_size) override;
    void close() override;

public:
    void pause(bool b) override;
};

}
