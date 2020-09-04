#include "input_base.h"

#include <libretro.h>

#include <spdlog/spdlog.h>

namespace drivers {

int16_t input_base::input_state(unsigned port, unsigned device, unsigned index, unsigned id) {
    auto &p = port == 0xFF ? port_menu : ports[port];
    if (!p.enabled) return 0;
    switch (device) {
    case RETRO_DEVICE_JOYPAD:
        if (id == RETRO_DEVICE_ID_JOYPAD_MASK) {
            return p.states;
        }
        return (p.states & (1 << id)) ? 1 : 0;
    case RETRO_DEVICE_KEYBOARD:
        return 0;
    case RETRO_DEVICE_ANALOG:
        if (index <= RETRO_DEVICE_INDEX_ANALOG_RIGHT) {
            if (id <= RETRO_DEVICE_ID_ANALOG_Y)
                return p.analog_axis[index][id];
        } else if (index == RETRO_DEVICE_INDEX_ANALOG_BUTTON) {
            if (id == RETRO_DEVICE_ID_JOYPAD_MASK) {
                return p.states;
            }
            return (p.states & (1 << id)) ? 1 : 0;
        }
        return 0;
    default:
        return 0;
    }
}

void input_base::add_button_desc(uint8_t port, uint8_t device, uint8_t index, uint16_t id, const std::string &desc) {
    output_button_t bt = {id, index, port, desc };
    if (port < 0xFF && port >= ports.size()) {
        ports.resize(port + 1, {});
    }
    auto &p = port == 0xFF ? port_menu : ports[port];
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
    auto &rev_mapping = port == 0xFFu ? rev_menu_mapping : rev_game_mapping;
    auto ite = rev_mapping.find((static_cast<uint64_t>(port) << 32) | static_cast<uint64_t>(value));
    if (ite != rev_mapping.end()) {
        auto &mapping = port == 0xFFu ? menu_mapping : game_mapping;
        mapping[ite->second] = &p.buttons[value];
        if (!p.enabled) p.enabled = true;
    }
}

void input_base::clear_button_desc() {
    game_mapping.clear();
    ports.clear();
}

void input_base::clear_menu_button_desc() {
    menu_mapping.clear();
    port_menu.buttons.clear();
    port_menu.device = port_menu.states = 0;
    port_menu.available = port_menu.enabled = false;
}

void input_base::add_mapping(uint64_t from, uint8_t to_port, uint16_t to_id) {
    if (to_port == 0xFF || to_port < ports.size()) {
        auto &port = to_port == 0xFF ? port_menu : ports[to_port];
        auto ite = port.buttons.find(to_id);
        if (ite != port.buttons.end()) {
            auto &mapping = to_port == 0xFFu ? menu_mapping : game_mapping;
            mapping[from] = &ite->second;
            if (!port.enabled) port.enabled = true;
            return;
        }
    }
    auto &rev_mapping = to_port == 0xFFu ? rev_menu_mapping : rev_game_mapping;
    rev_mapping[(static_cast<uint64_t>(to_port) << 32) | static_cast<uint64_t>(to_id)] = from;
}

void input_base::on_input(uint64_t id, bool pressed) {
    auto &mapping = in_menu ? menu_mapping : game_mapping;
    auto ite = mapping.find(id);
    if (ite == mapping.end()) return;
    auto *btn = ite->second;
    if (btn == nullptr) return;
    auto &port = btn->port == 0xFF ? port_menu : ports[btn->port];
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
