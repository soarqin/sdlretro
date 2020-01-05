#include "input_base.h"

#include <libretro.h>

namespace drivers {

void input_base::add_button(unsigned port, unsigned device, unsigned index, unsigned id, const std::string &desc) {
    input_button_t bt = { true, port, device, index, id, desc };
    if (port >= ports.size()) {
        ports.resize(port + 1, {});
    }
    auto &p = ports[port];
    p.available = true;
    if (id >= p.buttons.size()) p.buttons.resize(id + 1);
    p.buttons[id] = bt;
}

int16_t input_base::input_state(unsigned port, unsigned device, unsigned index, unsigned id) {
    switch (device) {
        case RETRO_DEVICE_JOYPAD:
            if (id==RETRO_DEVICE_ID_JOYPAD_MASK) {
                return pad_states[port];
            }
            return (pad_states[port] & (1 << id)) ? 1 : 0;
        default:
            return 0;
    }

}

}
