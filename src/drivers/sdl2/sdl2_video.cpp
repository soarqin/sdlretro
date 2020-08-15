#include "sdl2_video.h"

namespace drivers {

sdl2_video::sdl2_video() {

}
sdl2_video::~sdl2_video() {

}
bool sdl2_video::resolution_changed(unsigned int width, unsigned int height, unsigned int bpp) {
    return false;
}
void sdl2_video::render(const void *data, unsigned int width, unsigned int height, size_t pitch) {

}
void *sdl2_video::get_framebuffer(unsigned int *width, unsigned int *height, size_t *pitch, int *format) {
    return video_base::get_framebuffer(width, height, pitch, format);
}
void sdl2_video::clear() {

}
void sdl2_video::flip() {

}
void sdl2_video::draw_text(int x, int y, const char *text, int width, bool shadow) {

}
uint32_t sdl2_video::get_text_width(const char *text) {
    return 0;
}
void sdl2_video::draw_text_pixel(int x, int y, const char *text, int width, bool shadow) {

}
void sdl2_video::enter_menu() {

}
void sdl2_video::leave_menu() {

}
}
