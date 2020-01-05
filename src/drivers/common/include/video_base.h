#pragma once

#include <string>
#include <cstdlib>

namespace drivers {

class video_base {
public:
    virtual ~video_base() = default;

    virtual bool resolution_changed(unsigned width, unsigned height, unsigned bpp) = 0;
    virtual void render(const void *data, unsigned width, unsigned height, size_t pitch) = 0;

    virtual void *get_framebuffer(unsigned *width, unsigned *height, size_t *pitch, int *format)
    { return nullptr; }

    virtual bool frame_drawn() { return false; }

    inline void set_message(const char *text, unsigned frames) {
        message_text = text;
        message_frames = frames;
    }
    inline void message_frame_pass() {
        if (message_frames && --message_frames == 0) {
            message_text.clear();
        }
    }

protected:
    std::string message_text;
    unsigned message_frames;
};

}
