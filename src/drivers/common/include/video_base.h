#pragma once

#include <string>
#include <cstdlib>

namespace drivers {

class video_base {
public:
    virtual ~video_base() = default;

    /* pixel_format follows libretro: RGB1555=0 XRGB8888=1 RGB565=2 */
    virtual bool resolution_changed(unsigned width, unsigned height, unsigned pixel_format) = 0;
    virtual void render(const void *data, unsigned width, unsigned height, size_t pitch) = 0;

    virtual void enter_menu() {};
    virtual void leave_menu() {};

    virtual void *get_framebuffer(unsigned *width, unsigned *height, size_t *pitch, int *format)
    { return nullptr; }
    virtual bool frame_drawn() = 0;
    virtual void get_resolution(int &width, int &height) {}

    /* width: 0=fullscreen -1=fullscreen allow wrap
     *        others: negative = allow wrap */
    virtual void draw_text(int x, int y, const char *text, int width, bool shadow) {}
    virtual uint32_t get_text_width(const char *text) const { return 0; }

    virtual void clear() {}
    virtual void flip() {}

    inline void set_message(const char *text, unsigned frames) {
        message_text = text;
        message_frames = frames;
    }
    inline void message_frame_pass() {
        if (message_frames && --message_frames == 0) {
            message_text.clear();
        }
    }
    inline void set_skip_frame() { skip_frame = true; }
    inline void set_aspect_ratio(float ratio) { aspect_ratio = ratio; }

protected:
    bool skip_frame = false;
    std::string message_text;
    unsigned message_frames = 0;
    float aspect_ratio = 0.f;
};

}
