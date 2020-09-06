#include "video_base.h"

namespace drivers {

void video_base::add_message(const char *text, uint32_t frames) {
    if (frames)
        messages.emplace_back(text, frames);
}

void video_base::message_frame_pass() {
    for (auto ite = messages.begin(); ite != messages.end();) {
        if (--ite->second == 0) {
            ite = messages.erase(ite);
        } else {
            ++ite;
        }
    }
}

}
