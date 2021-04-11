#include "sdl_elem.h"

#include <video_base.h>
#include <driver_base.h>

namespace gui {

void sdl_progressbar::draw() {
    auto *video = driver->get_video();
    auto prog_w = prog_max > 0 ? static_cast<int>(static_cast<int64_t>(width - 4) * progress / prog_max) : 0;
    video->set_draw_color(box_color[0], box_color[1], box_color[2], box_color[3]);
    video->draw_rectangle(pos_x, pos_y, width, height);
    if (prog_w > 0) {
        video->set_draw_color(bar_color[0], bar_color[1], bar_color[2], bar_color[3]);
        video->fill_rectangle(pos_x + 2, pos_y + 2, prog_w, height - 4);
    }
}

}
