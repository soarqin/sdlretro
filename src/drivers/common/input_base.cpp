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
    }
    switch(device) {
    case RETRO_DEVICE_JOYPAD:
        index = RETRO_DEVICE_INDEX_ANALOG_BUTTON;
        break;
    default:
        break;
    }
    auto value = button_packed_value(index, id);
    p.available = true;
    p.buttons[value] = std::move(bt);
    for (auto &m: key_mapping) {
        if (std::get<0>(m.second) == port && std::get<1>(m.second) == value) {
            std::get<2>(m.second) = &p.buttons[value];
        }
    }
}

void input_base::clear_button_desc() {
    for (auto &m: key_mapping) {
        std::get<2>(m.second) = nullptr;
    }
    ports.clear();
}

void input_base::add_mapping(uint32_t from, uint8_t to_port, uint16_t to_id) {
    if (to_port < ports.size()) {
        auto &port = ports[to_port];
        auto ite = port.buttons.find(to_id);
        if (ite != port.buttons.end()) {
            key_mapping[from] = std::make_tuple(to_port, to_id, &ite->second);
            return;
        }
    }
    key_mapping[from] = std::make_tuple(to_port, to_id, nullptr);
}

void input_base::on_input(uint32_t id, bool pressed) {
    auto ite = key_mapping.find(id);
    if (ite == key_mapping.end()) return;
    auto *btn = std::get<2>(ite->second);
    if (btn == nullptr) return;
    switch(btn->index) {
    case RETRO_DEVICE_INDEX_ANALOG_BUTTON:
        if (pressed)
            ports[btn->port].states |= 1U << btn->id;
        else
            ports[btn->port].states &= ~(1U << btn->id);
        break;
    case RETRO_DEVICE_INDEX_ANALOG_LEFT:
    case RETRO_DEVICE_INDEX_ANALOG_RIGHT:
        ports[btn->port].analog_axis[btn->index][btn->id & 1] = pressed ? ((btn->id >> 1) ? -0x8000 : 0x7FFF) : 0;
        break;
    default:
        break;
    }
}

}
