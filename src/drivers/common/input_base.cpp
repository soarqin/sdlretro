#include "input_base.h"

#include <libretro.h>

#include <spdlog/spdlog.h>

namespace drivers {

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

void input_base::add_button_desc(uint8_t port, uint8_t device, uint8_t index, uint16_t id, const std::string &desc) {
    output_button_t bt = {id, index, port, true, desc };
    if (port >= ports.size()) {
        ports.resize(port + 1, {});
    }
    auto &p = ports[port];
    if (!p.available) {
        p.available = true;
    }
    if (p.device != device) {
        if (p.device != 0)
            spdlog::log(spdlog::level::warn, "Got different device type for port {}: original is {}, new is {}", port, p.device, device);
        p.device = device;
    }
    if (device == RETRO_DEVICE_ANALOG) {
        /* remap device index like this:
         *   RETRO_DEVICE_INDEX_ANALOG_BUTTON -> 0
         *   RETRO_DEVICE_INDEX_ANALOG_LEFT   -> 1
         *   RETRO_DEVICE_INDEX_ANALOG_RIGHT  -> 2
         */
        if (index < RETRO_DEVICE_INDEX_ANALOG_BUTTON) {
            ++index;
        } else if (index == RETRO_DEVICE_INDEX_ANALOG_BUTTON) {
            index = 0;
        }
    }
    auto value = button_packed_value(index, id);
    p.available = true;
    p.buttons[value] = std::move(bt);
    auto ite = rev_key_mapping.find((static_cast<uint64_t>(port) << 32) | static_cast<uint64_t>(value));
    if (ite != rev_key_mapping.end()) {
        key_mapping[ite->second] = &p.buttons[value];
        if (!p.enabled) p.enabled = true;
    }
}

void input_base::clear_button_desc() {
    key_mapping.clear();
    ports.clear();
}

void input_base::add_mapping(uint64_t from, uint8_t to_port, uint16_t to_id) {
    if (to_port < ports.size()) {
        auto &port = ports[to_port];
        auto ite = port.buttons.find(to_id);
        if (ite != port.buttons.end()) {
            key_mapping[from] = &ite->second;
            if (!port.enabled) port.enabled = true;
            return;
        }
    }
    rev_key_mapping[(static_cast<uint64_t>(to_port) << 32) | static_cast<uint64_t>(to_id)] = from;
}

void input_base::on_input(uint64_t id, bool pressed) {
    auto ite = key_mapping.find(id);
    if (ite == key_mapping.end()) return;
    auto *btn = ite->second;
    if (btn == nullptr) return;
    auto &port = ports[btn->port];
    switch(port.device) {
    case RETRO_DEVICE_JOYPAD:
        if (pressed)
            port.states |= 1U << btn->id;
        else
            port.states &= ~(1U << btn->id);
        break;
    case RETRO_DEVICE_ANALOG:
        /* process remapped index */
        if (btn->index == 0) {
            if (pressed)
                port.states |= 1U << btn->id;
            else
                port.states &= ~(1U << btn->id);
        } else {
            if (btn->index <= RETRO_DEVICE_INDEX_ANALOG_BUTTON) {
                port.analog_axis[btn->index - 1][btn->id & 1] = pressed ? ((btn->id >> 1) ? -0x8000 : 0x7FFF) : 0;
            }
        }
        break;
    default:
        break;
    }
}

}
