#include "input_base.h"

#include <libretro.h>

namespace drivers {

void input_base::add_button_desc(uint8_t port, uint8_t device, uint8_t index, uint16_t id, const std::string &desc) {
    input_button_t bt = { id, index, device, true, desc };
    if (port >= ports.size()) {
        ports.resize(port + 1, {});
        ports[0].enabled = true;
    }
    auto &p = ports[port];
    p.available = true;
    p.buttons[bt.value] = std::move(bt);
}

void input_base::map_button(uint32_t from, uint8_t to_port, uint32_t to_id) {
    key_mapping[from] = std::make_tuple(to_port, to_id);
}

int16_t input_base::input_state(unsigned port, unsigned device, unsigned index, unsigned id) {
    if (!ports[port].enabled) return 0;
    switch (device) {
        case RETRO_DEVICE_JOYPAD:
            if (id == RETRO_DEVICE_ID_JOYPAD_MASK) {
                return ports[port].states;
            }
            return (ports[port].states & (1 << id)) ? 1 : 0;
        case RETRO_DEVICE_KEYBOARD:
            return 0;
        case RETRO_DEVICE_ANALOG:
            if (index <= RETRO_DEVICE_INDEX_ANALOG_RIGHT) {
                if (id <= RETRO_DEVICE_ID_ANALOG_Y)
                    return ports[port].analog_axis[index][id];
            } else if (index == RETRO_DEVICE_INDEX_ANALOG_BUTTON) {
                if (id == RETRO_DEVICE_ID_JOYPAD_MASK) {
                    return ports[port].states;
                }
                return (ports[port].states & (1 << id)) ? 1 : 0;
            }
            return 0;
        default:
            return 0;
    }
}

void input_base::on_keydown(uint16_t id) {

}
void input_base::on_keyup(uint16_t id) {

}
void input_base::on_mousedown(uint16_t id) {

}
void input_base::on_mouseup(uint16_t id) {

}
void input_base::on_joybtndown(uint8_t port, uint16_t id) {

}
void input_base::on_joybtnup(uint8_t port, uint16_t id) {

}

}
