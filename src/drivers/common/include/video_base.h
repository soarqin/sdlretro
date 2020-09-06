#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstdlib>

namespace drivers {

class video_base {
public:
    virtual ~video_base() = default;

    /* pixel_format follows libretro: RGB1555=0 XRGB8888=1 RGB565=2 */
    virtual bool resolution_changed(unsigned width, unsigned height, unsigned pixel_format) = 0;
    virtual void render(const void *data, unsigned width, unsigned height, size_t pitch) = 0;

    virtual void enter_menu() {}
    virtual void leave_menu() {}
    virtual void predraw_menu() {}
    virtual void config_changed() {}

    virtual void *get_framebuffer(unsigned *width, unsigned *height, size_t *pitch, int *format)
    { return nullptr; }
    virtual bool frame_drawn() = 0;
    virtual void get_resolution(int &width, int &height) {}

    virtual int get_font_size() const { return 0; }
    virtual void set_draw_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { }
    virtual void draw_rectangle(int x, int y, int w, int h) {}
    virtual void fill_rectangle(int x, int y, int w, int h) {}
    /* width: 0=fullscreen -1=fullscreen allow wrap
     *        others: negative = allow wrap */
    virtual void draw_text(int x, int y, const char *text, int width, bool shadow) {}
    virtual void get_text_width_and_height(const char *text, uint32_t &w, int &t, int &b) const { }

    virtual void clear() {}
    virtual void flip() {}

    void add_message(const char *text, uint32_t frames);
    void message_frame_pass();
    inline void set_skip_frame() { skip_frame = true; }
    inline void set_aspect_ratio(float ratio) { aspect_ratio = ratio; }

protected:
    bool skip_frame = false;
    std::vector<std::pair<std::string, uint32_t>> messages;
    float aspect_ratio = 0.f;
};

}
